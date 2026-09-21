#include "texture.hpp"
#include "../core/gl_config.hpp"
#include "../utils/logger.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

// Include stb_image for image loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/**
 * @brief Constructor for the OkTexture class.
 *        Loads the texture from the specified path.
 * @param path The path to the texture file.
 */
OkTexture::OkTexture(const std::string &path) {
  this->path   = path;
  loaded       = false;
  coverageKept = false;
  id           = 0;
  width        = 0;
  height       = 0;
  channels     = 0;

  OkLogger::info("Texture", "Loading texture: " + path);

  // Load image data
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!data) {
    OkLogger::error("Texture", "Failed to load texture: " + path + " (" +
                                   std::string(stbi_failure_reason()) + ")");
    return;
  }

  // Create OpenGL texture
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load texture data
  GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0,
               format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free image data
  stbi_image_free(data);

  loaded = true;
}

/**
 * @brief Constructor for creating an empty texture with specified dimensions.
 * @param width Width of the texture.
 * @param height Height of the texture.
 * @param channels Number of color channels (3 for RGB, 4 for RGBA).
 */
OkTexture::OkTexture(int width, int height, int channels) {
  this->path         = "";  // No file path for raw textures
  this->width        = width;
  this->height       = height;
  this->channels     = channels;
  this->loaded       = false;
  this->coverageKept = false;
  this->id           = 0;

  // Create OpenGL texture
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/**
 * @brief Constructor for creating and initializing texture from raw data.
 * @param data Pointer to raw pixel data
 * @param width Width of the texture
 * @param height Height of the texture
 * @param channels Number of color channels (3 for RGB, 4 for RGBA)
 */
OkTexture::OkTexture(const unsigned char *data, int width, int height,
                     int channels) {
  this->path         = "";  // No file path for raw textures
  this->width        = width;
  this->height       = height;
  this->channels     = channels;
  this->loaded       = false;
  this->coverageKept = false;
  this->id           = 0;

  // Create OpenGL texture
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load the raw data
  GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0,
               format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  loaded = true;
}

/**
 * @brief Destructor for the OkTexture class.
 *        Cleans up the texture resources.
 */
OkTexture::~OkTexture() {
  if (loaded) {
    glDeleteTextures(1, &id);
  }
}

/**
 * @brief Binds the texture for rendering.
 */
/**
 * @brief Switch this texture to nearest min/mag filtering (mipmaps off):
 *        the right sampling for pixel-art content like the glyph atlas.
 */
void OkTexture::setNearestFiltering() const {
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void OkTexture::setMaxMipLevel(int level) const {
  // The full chain of a texture as large as GL allows, far past any
  // real one: what GL itself starts a texture's maximum level at.
  const GLint FULL_CHAIN = 1000;
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                  level < 0 ? FULL_CHAIN : static_cast<GLint>(level));
  glBindTexture(GL_TEXTURE_2D, 0);
}

namespace {

  // Bytes per texel of the images the coverage code works on.
  const int RGBA_BYTES = 4;
  // Where alpha sits in a texel, and its full value.
  const int   ALPHA_OFFSET = 3;
  const float ALPHA_FULL   = 255.0f;

}  // namespace

float OkTexture::alphaCoverage(const std::vector<unsigned char> &alphas,
                               float threshold, float scale) {
  if (alphas.empty()) {
    return 0.0f;
  }
  size_t passed = 0;
  for (size_t i = 0; i < alphas.size(); i++) {
    float a = static_cast<float>(alphas[i]) / ALPHA_FULL * scale;
    if (std::min(a, 1.0f) >= threshold) {
      passed++;
    }
  }
  return static_cast<float>(passed) / static_cast<float>(alphas.size());
}

float OkTexture::coverageScale(const std::vector<unsigned char> &alphas,
                               float threshold, float target) {
  size_t texels = alphas.size();
  if (texels == 0 || alphaCoverage(alphas, threshold, 1.0f) >= target) {
    return 1.0f;
  }
  // The texels that have to pass are the brightest `wanted`, so the
  // scale is whatever lifts the dimmest of those to the threshold:
  // found by ranking the alphas, not by searching over scales.
  size_t wanted =
      static_cast<size_t>(std::lround(target * static_cast<float>(texels)));
  if (wanted == 0) {
    return 1.0f;
  }
  wanted                            = std::min(wanted, texels);
  std::vector<unsigned char> ranked = alphas;
  std::nth_element(ranked.begin(),
                   ranked.begin() + static_cast<long>(wanted - 1), ranked.end(),
                   std::greater<>());
  unsigned char dimmest = ranked[wanted - 1];
  if (dimmest == 0) {
    // Fewer texels hold any alpha at all than have to pass: lift every
    // one of them, which is as close as scaling can come.
    unsigned char least = 255;
    for (size_t i = 0; i < texels; i++) {
      if (ranked[i] > 0) {
        least = std::min(least, ranked[i]);
      }
    }
    dimmest = least;
  }
  float scale = threshold * ALPHA_FULL / static_cast<float>(dimmest);
  return std::max(scale, 1.0f);
}

void OkTexture::halveForCutout(const std::vector<unsigned char> &src, int width,
                               int height, std::vector<unsigned char> &dst,
                               int &outWidth, int &outHeight) {
  outWidth  = std::max(width / 2, 1);
  outHeight = std::max(height / 2, 1);
  dst.assign(static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight) *
                 RGBA_BYTES,
             0);
  for (int y = 0; y < outHeight; y++) {
    for (int x = 0; x < outWidth; x++) {
      std::array<float, 3> colour = {0.0f, 0.0f, 0.0f};
      std::array<float, 3> plain  = {0.0f, 0.0f, 0.0f};
      float                alpha  = 0.0f;
      for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
          int    sx = std::min(x * 2 + dx, width - 1);
          int    sy = std::min(y * 2 + dy, height - 1);
          size_t si = (static_cast<size_t>(sy) * static_cast<size_t>(width) +
                       static_cast<size_t>(sx)) *
                      RGBA_BYTES;
          float  a  = static_cast<float>(src[si + ALPHA_OFFSET]);
          for (int c = 0; c < 3; c++) {
            colour[static_cast<size_t>(c)] +=
                static_cast<float>(src[si + static_cast<size_t>(c)]) * a;
            plain[static_cast<size_t>(c)] +=
                static_cast<float>(src[si + static_cast<size_t>(c)]);
          }
          alpha += a;
        }
      }
      size_t di = (static_cast<size_t>(y) * static_cast<size_t>(outWidth) +
                   static_cast<size_t>(x)) *
                  RGBA_BYTES;
      for (int c = 0; c < 3; c++) {
        // Weighted by coverage, so an empty pixel lends the average
        // nothing; with none covered, the plain mean.
        float v = alpha > 0.0f ? colour[static_cast<size_t>(c)] / alpha
                               : plain[static_cast<size_t>(c)] / 4.0f;
        dst[di + static_cast<size_t>(c)] =
            static_cast<unsigned char>(std::min(v + 0.5f, ALPHA_FULL));
      }
      dst[di + ALPHA_OFFSET] =
          static_cast<unsigned char>(std::min(alpha / 4.0f + 0.5f, ALPHA_FULL));
    }
  }
}

namespace {

  /**
   * @brief Which square a texel of a mipmap level falls in, counted in
   *        the full image.
   *
   * @param step How many texels of the full image one texel of this
   *             level spans on a side: 2 to the power of the level.
   */
  size_t squareOf(int x, int y, int step, int side, int cols, int rows) {
    int col = std::min(x * step / side, cols - 1);
    int row = std::min(y * step / side, rows - 1);
    return static_cast<size_t>(row) * static_cast<size_t>(cols) +
           static_cast<size_t>(col);
  }

  /**
   * @brief The alphas of an RGBA image, sorted into the squares they fall
   *        in.
   *
   * @param step  How many texels of the full image one texel of this
   *              image spans on a side.
   * @param side  The squares' side in texels of the full image, which
   *              is `cols` by `rows` of them, the last ones cut short.
   */
  std::vector<std::vector<unsigned char>>
  alphasBySquare(const std::vector<unsigned char> &rgba, int width, int height,
                 int step, int side, int cols, int rows) {
    std::vector<std::vector<unsigned char>> squares(static_cast<size_t>(cols) *
                                                    static_cast<size_t>(rows));
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        size_t square = squareOf(x, y, step, side, cols, rows);
        size_t texel  = (static_cast<size_t>(y) * static_cast<size_t>(width) +
                         static_cast<size_t>(x)) *
                        RGBA_BYTES;
        squares[square].push_back(rgba[texel + ALPHA_OFFSET]);
      }
    }
    return squares;
  }

}  // namespace

void OkTexture::keepCutoutCoverage(float threshold, int region) {
  if (!loaded || coverageKept || channels != RGBA_BYTES || width <= 0 ||
      height <= 0) {
    return;
  }
  // It runs on the thread that owns the context, so what it costs is
  // said: a frame that pays for it should be able to find out why.
  std::chrono::steady_clock::time_point started =
      std::chrono::steady_clock::now();
  // Rows of RGBA are whole words anyway; set to one so nothing depends
  // on that, and put back as found for whoever reads or writes next.
  GLint packWas   = 4;
  GLint unpackWas = 4;
  glGetIntegerv(GL_PACK_ALIGNMENT, &packWas);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackWas);
  std::vector<unsigned char> level(static_cast<size_t>(width) *
                                   static_cast<size_t>(height) * RGBA_BYTES);
  glBindTexture(GL_TEXTURE_2D, id);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, level.data());

  // The squares, counted in the full image, and the share of each that
  // the full image covers: what every level is held to.
  int side = region > 0 ? region : std::max(width, height);
  int cols = (width + side - 1) / side;
  int rows = (height + side - 1) / side;
  std::vector<std::vector<unsigned char>> full =
      alphasBySquare(level, width, height, 1, side, cols, rows);
  std::vector<float> target(full.size());
  for (size_t i = 0; i < full.size(); i++) {
    target[i] = alphaCoverage(full[i], threshold, 1.0f);
  }

  int                        w     = width;
  int                        h     = height;
  int                        index = 0;
  int                        step  = 1;
  std::vector<unsigned char> next;
  std::vector<unsigned char> scaled;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  while (w > 1 || h > 1) {
    int nw = 0;
    int nh = 0;
    halveForCutout(level, w, h, next, nw, nh);
    index++;
    step *= 2;
    std::vector<std::vector<unsigned char>> squares =
        alphasBySquare(next, nw, nh, step, side, cols, rows);
    std::vector<float> scale(squares.size());
    for (size_t i = 0; i < squares.size(); i++) {
      scale[i] = coverageScale(squares[i], threshold, target[i]);
    }
    // The next level is built from this one unscaled: a scale is this
    // level's answer to its own loss, and handed down it would be
    // applied again on top of the loss below.
    scaled = next;
    for (int y = 0; y < nh; y++) {
      for (int x = 0; x < nw; x++) {
        size_t square = squareOf(x, y, step, side, cols, rows);
        if (scale[square] <= 1.0f) {
          continue;
        }
        size_t texel = (static_cast<size_t>(y) * static_cast<size_t>(nw) +
                        static_cast<size_t>(x)) *
                           RGBA_BYTES +
                       ALPHA_OFFSET;
        float  a     = static_cast<float>(scaled[texel]) * scale[square];
        scaled[texel] =
            static_cast<unsigned char>(std::min(a + 0.5f, ALPHA_FULL));
      }
    }
    glTexImage2D(GL_TEXTURE_2D, index, GL_RGBA, nw, nh, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, scaled.data());
    level.swap(next);
    w = nw;
    h = nh;
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, unpackWas);
  glPixelStorei(GL_PACK_ALIGNMENT, packWas);
  glBindTexture(GL_TEXTURE_2D, 0);
  coverageKept = true;
  double ms    = std::chrono::duration<double, std::milli>(
                     std::chrono::steady_clock::now() - started)
                     .count();
  OkLogger::info("Texture", "Cutout coverage kept over " +
                                std::to_string(index) + " mipmap levels of " +
                                path + " in " + std::to_string(ms) + " ms");
}

/**
 * @brief Replace the texture's pixel data in place (dimensions must match
 *        the ones it was created with). Used by dynamic textures like the
 *        skybox gradient.
 */
void OkTexture::updateRawData(const unsigned char *data, int newWidth,
                              int newHeight) const {
  if (!loaded || newWidth != width || newHeight != height) {
    return;
  }
  GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
  glBindTexture(GL_TEXTURE_2D, id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format,
                  GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void OkTexture::bind() const {
  if (loaded) {
    glBindTexture(GL_TEXTURE_2D, id);
  }
}

/**
 * @brief Unbinds the currently bound texture.
 */
void OkTexture::unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief Creates a texture from raw data.
 * @param data Pointer to the raw image data.
 * @param width Width of the texture.
 * @param height Height of the texture.
 * @param format Format of the texture (GL_RGBA, GL_RGB, etc.).
 * @param internalFormat Internal format of the texture (default: GL_RGBA).
 * @return True if the texture was created successfully, false otherwise.
 */
bool OkTexture::createFromRawData(const unsigned char *data, int width,
                                  int height, GLenum format,
                                  GLenum internalFormat) {
  if (!data || width <= 0 || height <= 0) {
    return false;
  }

  this->width    = width;
  this->height   = height;
  this->channels = format == GL_RGBA ? 4 : 3;

  if (!loaded) {
    glGenTextures(1, &id);
  }

  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), width,
               height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  // New pixels, new ordinary mipmaps: whatever keepCutoutCoverage() did
  // was for the image that was here before.
  coverageKept = false;

  loaded = true;
  return true;
}
