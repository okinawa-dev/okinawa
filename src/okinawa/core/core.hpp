#ifndef OK_CORE_HPP
#define OK_CORE_HPP

#include "../handlers/scenes.hpp"
#include "../input/input.hpp"
#include "./camera.hpp"
#include "gl_config.hpp"
#include <functional>
#include <vector>

// Forward declaration: the MCP server type is only a pointer here, and its
// full definition (and dependencies) stays out of this public header.
class OkMcpServer;

// Forward declaration: the active avatar is tracked as a pointer (not owned).
class OkAvatar;

/**
 * @brief Core class for the Okinawa engine.
 *        It handles the initialization of OpenGL, shaders, and the main loop.
 */
class OkCore {
public:
  // Callback type for engine loop
  using OkCoreCallback = std::function<void(float deltaTime)>;

  // Delete constructor to prevent instantiation
  OkCore() = delete;

  // Core initialization and loop
  static bool initialize();
  static void loop(const OkCoreCallback &stepCallback,
                   const OkCoreCallback &drawCallback);
  static void askForExit();
  static void exit();

  // Scene handler
  static OkSceneHandler *getSceneHandler() { return _sceneHandler; }

  // Getters
  static OkCamera   *getCamera() { return _cameras[_currentCamera]; }
  static GLFWwindow *getWindow() { return _window; }
  static GLuint      getShaderProgram() { return _shaderProgram; }
  static OkInput    *getInput() { return _input; }

  // Camera management
  static void addCamera(OkCamera *camera);
  static void switchCamera(int index);
  static int  getCurrentCameraIndex() { return _currentCamera; }
  static int  getCameraCount() { return static_cast<int>(_cameras.size()); }

  // Active avatar: the controlled entity that receives input each frame. Not
  // owned by the core (the game keeps ownership). Set null to fall back to the
  // free-fly camera control. Swapping it (on foot -> car) changes the controls
  // and camera behaviour in one call.
  static void      setActiveAvatar(OkAvatar *avatar) { _activeAvatar = avatar; }
  static OkAvatar *getActiveAvatar() { return _activeAvatar; }

  // Enable the in-engine MCP server so an external agent can connect over
  // local HTTP and drive the app (v1: capture the rendered frame). Binds
  // 127.0.0.1:port. This symbol always exists; if the engine was built
  // without MCP support (OKINAWA_WITH_MCP undefined) it logs a warning and
  // does nothing, so apps compile identically with or without the server.
  static void enableMcpServer(int port = 8765);

  // Ignore the user's physical keyboard/mouse input (e.g. when an instance is
  // meant to be driven only through the MCP server). Injected/MCP input still
  // works. Call after initialize().
  static void setIgnoreUserInput(bool ignore);

private:
  static bool initializeOpenGL(int width, int height);
  static bool initializeShaders();

  static GLFWwindow             *_window;
  static std::vector<OkCamera *> _cameras;
  static int                     _currentCamera;
  static OkSceneHandler         *_sceneHandler;
  static GLuint                  _shaderProgram;
  static OkInput                *_input;
  static OkMcpServer            *_mcpServer;
  static OkAvatar               *_activeAvatar;

  static void mouseCallback(GLFWwindow *window, double xpos, double ypos);
  // Keep the GL viewport matching the (possibly HiDPI) framebuffer size.
  static void framebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
};

#endif
