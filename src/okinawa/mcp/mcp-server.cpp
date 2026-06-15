#include "mcp-server.hpp"

#include "../core/core.hpp"  // OkCore::getWindow + OpenGL / GLFW headers
#include "../utils/logger.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

// stb_image_write provides the PNG encoder. The read side
// (STB_IMAGE_IMPLEMENTATION) is defined elsewhere (item/texture.cpp); the
// write side uses a different macro and header, so there is no conflict.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <chrono>
#include <cstring>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using nlohmann::json;

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

// Standard base64 encoder (no external dependency).
static std::string base64Encode(const std::vector<unsigned char> &data) {
  static const char table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve(((data.size() + 2) / 3) * 4);

  size_t i = 0;
  while (i + 2 < data.size()) {
    unsigned int n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
    out.push_back(table[(n >> 18) & 0x3F]);
    out.push_back(table[(n >> 12) & 0x3F]);
    out.push_back(table[(n >> 6) & 0x3F]);
    out.push_back(table[n & 0x3F]);
    i += 3;
  }

  size_t remaining = data.size() - i;
  if (remaining == 1) {
    unsigned int n = data[i] << 16;
    out.push_back(table[(n >> 18) & 0x3F]);
    out.push_back(table[(n >> 12) & 0x3F]);
    out.push_back('=');
    out.push_back('=');
  } else if (remaining == 2) {
    unsigned int n = (data[i] << 16) | (data[i + 1] << 8);
    out.push_back(table[(n >> 18) & 0x3F]);
    out.push_back(table[(n >> 12) & 0x3F]);
    out.push_back(table[(n >> 6) & 0x3F]);
    out.push_back('=');
  }
  return out;
}

// stb_image_write callback that appends encoded bytes to a vector.
static void stbWriteToVector(void *context, void *data, int size) {
  std::vector<unsigned char> *out =
      static_cast<std::vector<unsigned char> *>(context);
  unsigned char *bytes = static_cast<unsigned char *>(data);
  out->insert(out->end(), bytes, bytes + size);
}

// Result of a framebuffer capture (filled on the GL thread).
struct CaptureResult {
  bool        ok = false;
  std::string error;
  std::string base64Png;
  int         width  = 0;
  int         height = 0;
};

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct OkMcpServer::Impl {
  int                               port;
  httplib::Server                   server;
  std::thread                       thread;
  std::mutex                        queueMutex;
  std::deque<std::function<void()>> queue;

  // Enqueue work to run on the engine loop thread (via drainCommands).
  void enqueue(const std::function<void()> &fn) {
    std::lock_guard<std::mutex> lock(queueMutex);
    queue.push_back(fn);
  }

  // Capture the current framebuffer as a base64 PNG. Called from an HTTP
  // worker thread; the actual GL work is marshalled onto the loop thread.
  CaptureResult captureFrame() {
    std::shared_ptr<std::promise<CaptureResult>> promise =
        std::make_shared<std::promise<CaptureResult>>();
    std::future<CaptureResult> future = promise->get_future();

    enqueue([promise]() {
      CaptureResult result;
      GLFWwindow   *window = OkCore::getWindow();
      if (window == nullptr) {
        result.error = "no window";
        promise->set_value(result);
        return;
      }

      int width  = 0;
      int height = 0;
      glfwGetFramebufferSize(window, &width, &height);
      if (width <= 0 || height <= 0) {
        result.error = "invalid framebuffer size";
        promise->set_value(result);
        return;
      }

      int                        stride = width * 4;
      std::vector<unsigned char> pixels(static_cast<size_t>(stride) * height);
      glPixelStorei(GL_PACK_ALIGNMENT, 1);
      glReadBuffer(GL_BACK);
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                   pixels.data());

      // OpenGL's origin is bottom-left; PNG expects top-left, so flip rows.
      std::vector<unsigned char> flipped(static_cast<size_t>(stride) * height);
      for (int y = 0; y < height; y++) {
        memcpy(&flipped[static_cast<size_t>(stride) * (height - 1 - y)],
               &pixels[static_cast<size_t>(stride) * y], stride);
      }

      std::vector<unsigned char> png;
      int rc = stbi_write_png_to_func(stbWriteToVector, &png, width, height, 4,
                                      flipped.data(), stride);
      if (rc == 0 || png.empty()) {
        result.error = "png encoding failed";
        promise->set_value(result);
        return;
      }

      result.ok        = true;
      result.base64Png = base64Encode(png);
      result.width     = width;
      result.height    = height;
      promise->set_value(result);
    });

    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
      CaptureResult result;
      result.error = "capture timed out (is the app rendering?)";
      return result;
    }
    return future.get();
  }
};

// ---------------------------------------------------------------------------
// JSON-RPC / MCP handling
// ---------------------------------------------------------------------------

namespace {

const char *kProtocolVersion = "2024-11-05";
const char *kServerName      = "okinawa";
const char *kServerVersion   = "0.1.0";

json makeError(const json &id, int code, const std::string &message) {
  json response;
  response["jsonrpc"]          = "2.0";
  response["id"]               = id;
  response["error"]["code"]    = code;
  response["error"]["message"] = message;
  return response;
}

json makeResult(const json &id, const json &result) {
  json response;
  response["jsonrpc"] = "2.0";
  response["id"]      = id;
  response["result"]  = result;
  return response;
}

// The single tool exposed in v1.
json viewFrameToolSchema() {
  json tool;
  tool["name"] = "view_frame";
  tool["description"] =
      "Capture the current rendered frame of the okinawa app and return it "
      "as a PNG image, so the agent can visually inspect what is on screen.";
  tool["inputSchema"]["type"]                 = "object";
  tool["inputSchema"]["properties"]           = json::object();
  tool["inputSchema"]["additionalProperties"] = false;
  return tool;
}

}  // namespace

// ---------------------------------------------------------------------------
// OkMcpServer
// ---------------------------------------------------------------------------

OkMcpServer::OkMcpServer(int port) {
  _impl       = new Impl();
  _impl->port = port;
}

OkMcpServer::~OkMcpServer() {
  stop();
  delete _impl;
}

void OkMcpServer::start() {
  // Single MCP endpoint (Streamable HTTP): one POST that carries the
  // JSON-RPC request and returns a JSON-RPC response.
  _impl->server.Post(
      "/mcp", [this](const httplib::Request &req, httplib::Response &res) {
        json request = json::parse(req.body, nullptr, false);
        if (request.is_discarded() || !request.is_object()) {
          res.status = 200;
          res.set_content(makeError(nullptr, -32700, "parse error").dump(),
                          "application/json");
          return;
        }

        json id     = request.contains("id") ? request["id"] : json(nullptr);
        std::string method =
            request.value("method", std::string());
        json params = request.contains("params") ? request["params"]
                                                  : json::object();

        // Notifications (no id) get no response body.
        bool isNotification = !request.contains("id");

        if (method == "initialize") {
          json result;
          // Echo the client's protocol version when present.
          result["protocolVersion"] =
              params.value("protocolVersion", std::string(kProtocolVersion));
          result["capabilities"]["tools"] = json::object();
          result["serverInfo"]["name"]    = kServerName;
          result["serverInfo"]["version"] = kServerVersion;
          res.set_header("Mcp-Session-Id", "okinawa-mcp");
          res.set_content(makeResult(id, result).dump(), "application/json");
        } else if (method == "notifications/initialized" ||
                   method.rfind("notifications/", 0) == 0) {
          res.status = 202;
          res.set_content("", "application/json");
        } else if (method == "ping") {
          res.set_content(makeResult(id, json::object()).dump(),
                          "application/json");
        } else if (method == "tools/list") {
          json result;
          result["tools"] = json::array({viewFrameToolSchema()});
          res.set_content(makeResult(id, result).dump(), "application/json");
        } else if (method == "tools/call") {
          std::string name = params.value("name", std::string());
          if (name == "view_frame") {
            CaptureResult capture = _impl->captureFrame();
            json          result;
            if (capture.ok) {
              json image;
              image["type"]     = "image";
              image["data"]     = capture.base64Png;
              image["mimeType"] = "image/png";
              result["content"] = json::array({image});
              result["isError"] = false;
            } else {
              json text;
              text["type"]      = "text";
              text["text"]      = "view_frame failed: " + capture.error;
              result["content"] = json::array({text});
              result["isError"] = true;
            }
            res.set_content(makeResult(id, result).dump(), "application/json");
          } else {
            res.set_content(
                makeError(id, -32602, "unknown tool: " + name).dump(),
                "application/json");
          }
        } else if (isNotification) {
          res.status = 202;
          res.set_content("", "application/json");
        } else {
          res.set_content(
              makeError(id, -32601, "method not found: " + method).dump(),
              "application/json");
        }
      });

  _impl->thread = std::thread([this]() {
    bool ok = _impl->server.listen("127.0.0.1", _impl->port);
    if (!ok) {
      OkLogger::error("MCP", "Failed to bind MCP server to 127.0.0.1:" +
                                 std::to_string(_impl->port));
    }
  });
}

void OkMcpServer::stop() {
  if (_impl == nullptr) {
    return;
  }
  _impl->server.stop();
  if (_impl->thread.joinable()) {
    _impl->thread.join();
  }
}

void OkMcpServer::drainCommands() {
  std::deque<std::function<void()>> local;
  {
    std::lock_guard<std::mutex> lock(_impl->queueMutex);
    local.swap(_impl->queue);
  }
  for (size_t i = 0; i < local.size(); i++) {
    local[i]();
  }
}
