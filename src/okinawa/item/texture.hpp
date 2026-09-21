#ifndef OK_TEXTURE_HPP
#define OK_TEXTURE_HPP

#include "../core/gl_config.hpp"
#include <string>
#include <vector>

// The filtered alpha under which the world pass drops a pixel of an item
// drawn as a cutout (OkItem::setAlphaCutout). It lives in the shader as
// ALPHA_CUTOUT_THRESHOLD; this is the same number, for the code that
// prepares a texture to be cut at it, and the two must agree.
const float OK_ALPHA_CUTOUT_THRESHOLD = 0.5f;

/**
 * @brief OkTexture class for loading and managing textures.
 */
class OkTexture {
private:
  std::string path;
  bool        loaded;
  bool        coverageKept;  // mipmaps rebuilt by keepCutoutCoverage()
  GLuint      id;
  int         width;
  int         height;
  int         channels;

public:
  // Constructor loads the texture from file
  explicit OkTexture(const std::string &path);

  // Constructor for creating texture from raw data
  OkTexture(int width, int height, int channels);

  // Constructor for creating and initializing texture from raw data
  OkTexture(const unsigned char *data, int width, int height, int channels);

  // Destructor handles cleanup
  ~OkTexture();

  // Delete copy constructor and assignment
  OkTexture(const OkTexture &)            = delete;
  OkTexture &operator=(const OkTexture &) = delete;

  // Texture operations
  void        bind() const;
  static void unbind();

  // Switch to nearest filtering (no mipmaps): crisp pixel textures such
  // as glyph atlases, where linear sampling would bleed between cells.
  void setNearestFiltering() const;

  /**
   * @brief Stop minification at a mipmap level, and never go coarser.
   *
   * A texture that packs several images into one (an atlas) pads each
   * image with copies of its own edge, so a filter reaching past it
   * reads more of the same image. Each mipmap level doubles how far the
   * filter reaches, and past the level the padding covers, the filter
   * reads the neighbouring image or the empty space between: colours
   * bleed across and a solid image seen from afar or at a slant opens
   * holes along its edges. Capping the level at the last one the
   * padding covers trades that for a little aliasing at a great
   * distance, which is the smaller fault.
   *
   * @param level The coarsest level sampled; 0 is the full image. A
   *              negative value restores the whole chain.
   */
  void setMaxMipLevel(int level) const;

  /**
   * @brief Rebuild the mipmaps so an alpha cutout keeps its coverage at
   *        every level.
   *
   * A cutout is tested against a threshold, and the ordinary mipmaps
   * average it away: a grille of bars one pixel wide in six is a sixth
   * covered, so from the first level where a texel spans a bar and its
   * gap the average falls under one half and every bar is dropped. Seen
   * from a few tens of metres the grille is simply gone.
   *
   * This reads the full image back, builds each level from the one
   * above, and scales that level's alpha so the share of its texels that
   * pass the threshold is the share that passed in the full image -- the
   * grille thins out with distance instead of vanishing. Colour is
   * averaged over the covered texels only, so what shows at a distance
   * is the colour of the bars, not a mix of bars and whatever colour the
   * empty pixels carry. Alpha is only ever scaled up: a level that
   * covers more than the full image is left as it is, so nothing solid
   * is ever opened.
   *
   * The share is kept square by square, `region` texels of the full
   * image on a side, each with its own scale. One scale for a whole
   * atlas is decided by whatever else the atlas holds: the solid images
   * beside a grille gain coverage at a distance as their padding spreads,
   * that gain pays for the grille's loss, and the grille is dropped
   * anyway. An atlas wants squares about the size of its images.
   *
   * Done once per texture: later calls return at once. Needs the
   * texture's context current, an RGBA texture and a loaded one; it does
   * nothing otherwise.
   *
   * @param threshold The cutout threshold the texture will be drawn at.
   * @param region    Side of the squares kept apart, in texels of the
   *                  full image; 0 keeps the share over the whole image.
   */
  void keepCutoutCoverage(float threshold = OK_ALPHA_CUTOUT_THRESHOLD,
                          int   region    = 0);

  /**
   * @brief Share of a list of alphas that, times a scale and capped at
   *        one, reach a threshold.
   */
  static float alphaCoverage(const std::vector<unsigned char> &alphas,
                             float threshold, float scale);

  /**
   * @brief The scale, never under one, that brings a list of alphas'
   *        coverage at a threshold up to a target share.
   */
  static float coverageScale(const std::vector<unsigned char> &alphas,
                             float threshold, float target);

  /**
   * @brief One mipmap level from the one above: each texel the box of
   *        four under it, colour weighted by alpha.
   *
   * @param src  RGBA, `width` x `height`.
   * @param dst  Receives RGBA, `outWidth` x `outHeight`, half of each
   *             side and never under one.
   */
  static void halveForCutout(const std::vector<unsigned char> &src, int width,
                             int height, std::vector<unsigned char> &dst,
                             int &outWidth, int &outHeight);

  // Replace the pixel data in place (same size and channel count the
  // texture was created with). RGBA/RGB raw data, no mipmap rebuild.
  void updateRawData(const unsigned char *data, int newWidth,
                     int newHeight) const;
  bool isLoaded() const {
    return loaded;
  }

  // Getters
  int getWidth() const {
    return width;
  }
  int getHeight() const {
    return height;
  }
  int getChannels() const {
    return channels;
  }
  const std::string &getPath() const {
    return path;
  }

  // Create texture from raw data
  bool createFromRawData(const unsigned char *data, int width, int height,
                         GLenum format, GLenum internalFormat = GL_RGBA);
};

#endif
