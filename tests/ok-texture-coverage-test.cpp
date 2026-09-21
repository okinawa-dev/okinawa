#include "okinawa/item/texture.hpp"
#include "test-opengl.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

// Mipmaps that keep a cutout's coverage.
//
// The upload needs a context; the arithmetic under it does not, and it
// is where the faults would be: a grille that still vanishes at a
// distance, or a solid image opened up by a scale that went the wrong
// way. Held here on images small enough to reason about by hand.

namespace {

  // Side of the test images, in texels.
  const int SIDE = 32;
  // A bar every this many columns, one texel wide: a grille an eighth
  // covered, like a railing drawn one pixel of bar in several.
  const int BAR_EVERY = 8;
  // How much of the full image the grille covers.
  const float GRILLE_COVERAGE = 1.0f / static_cast<float>(BAR_EVERY);
  // Just under a scale, to show it is the least one that will do.
  const float JUST_UNDER = 0.99f;

  // A square RGBA image of vertical bars on nothing: the bars grey and
  // opaque, the gaps black and empty.
  std::vector<unsigned char> grille() {
    std::vector<unsigned char> img(static_cast<size_t>(SIDE * SIDE * 4), 0);
    for (int y = 0; y < SIDE; y++) {
      for (int x = 0; x < SIDE; x++) {
        if (x % BAR_EVERY == 0) {
          size_t i   = static_cast<size_t>(y * SIDE + x) * 4;
          img[i]     = 120;
          img[i + 1] = 120;
          img[i + 2] = 120;
          img[i + 3] = 255;
        }
      }
    }
    return img;
  }

  // The alphas of an RGBA image, which is what coverage is counted on.
  std::vector<unsigned char> alphasOf(const std::vector<unsigned char> &rgba) {
    std::vector<unsigned char> out;
    for (size_t i = 3; i < rgba.size(); i += 4) {
      out.push_back(rgba[i]);
    }
    return out;
  }

  // A square RGBA image covered everywhere.
  std::vector<unsigned char> solid() {
    std::vector<unsigned char> img(static_cast<size_t>(SIDE * SIDE * 4), 200);
    return img;
  }

}  // namespace

TEST_CASE("A grille averaged down loses its coverage, and the scale "
          "brings it back",
          "[texture][coverage]") {
  std::vector<unsigned char> full = grille();
  float                      target =
      OkTexture::alphaCoverage(alphasOf(full), OK_ALPHA_CUTOUT_THRESHOLD, 1.0f);
  REQUIRE(target == Catch::Approx(GRILLE_COVERAGE));

  // Two levels down a texel spans four columns, one of them a bar: a
  // quarter covered, under the threshold, so a plain mipmap drops every
  // bar in the image.
  std::vector<unsigned char> half;
  std::vector<unsigned char> quarter;
  int                        w = 0;
  int                        h = 0;
  OkTexture::halveForCutout(full, SIDE, SIDE, half, w, h);
  OkTexture::halveForCutout(half, w, h, quarter, w, h);
  REQUIRE(w == SIDE / 4);
  REQUIRE(h == SIDE / 4);
  REQUIRE(OkTexture::alphaCoverage(alphasOf(quarter), OK_ALPHA_CUTOUT_THRESHOLD,
                                   1.0f) == Catch::Approx(0.0f));

  // Rescaled, the level passes at least the share the full image did,
  // and the scale is the least that does it.
  float scale = OkTexture::coverageScale(alphasOf(quarter),
                                         OK_ALPHA_CUTOUT_THRESHOLD, target);
  REQUIRE(scale > 1.0f);
  REQUIRE(OkTexture::alphaCoverage(alphasOf(quarter), OK_ALPHA_CUTOUT_THRESHOLD,
                                   scale) >= target);
  REQUIRE(OkTexture::alphaCoverage(alphasOf(quarter), OK_ALPHA_CUTOUT_THRESHOLD,
                                   scale * JUST_UNDER) < target);
}

TEST_CASE("The colour of a level is the colour of what covers it",
          "[texture][coverage]") {
  std::vector<unsigned char> half;
  int                        w = 0;
  int                        h = 0;
  OkTexture::halveForCutout(grille(), SIDE, SIDE, half, w, h);
  // A bar and a black gap averaged plainly would be half as bright; the
  // empty gap lends nothing, so the texel keeps the bar's grey.
  REQUIRE(static_cast<int>(half[0]) == 120);
  REQUIRE(static_cast<int>(half[3]) == 128);
}

TEST_CASE("A solid image is never scaled, and never opened",
          "[texture][coverage]") {
  std::vector<unsigned char> full = solid();
  float                      target =
      OkTexture::alphaCoverage(alphasOf(full), OK_ALPHA_CUTOUT_THRESHOLD, 1.0f);
  std::vector<unsigned char> half;
  int                        w = 0;
  int                        h = 0;
  OkTexture::halveForCutout(full, SIDE, SIDE, half, w, h);
  REQUIRE(OkTexture::coverageScale(alphasOf(half), OK_ALPHA_CUTOUT_THRESHOLD,
                                   target) == Catch::Approx(1.0f));
  // Asked for less than it has, the scale stays at one rather than
  // dimming what is already solid.
  REQUIRE(OkTexture::coverageScale(alphasOf(half), OK_ALPHA_CUTOUT_THRESHOLD,
                                   0.1f) == Catch::Approx(1.0f));
}

TEST_CASE("A texture keeps its grille at a distance once asked to",
          "[texture][coverage]") {
  TestGLFWContext            context;
  std::vector<unsigned char> full = grille();
  OkTexture                  texture(full.data(), SIDE, SIDE, 4);
  REQUIRE(texture.isLoaded());
  texture.keepCutoutCoverage(OK_ALPHA_CUTOUT_THRESHOLD, 0);

  // Read the level where a plain mipmap had dropped every bar.
  const int                  LEVEL      = 2;
  const int                  LEVEL_SIDE = SIDE / 4;  // halved twice
  std::vector<unsigned char> quarter(
      static_cast<size_t>(LEVEL_SIDE * LEVEL_SIDE * 4));
  texture.bind();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glGetTexImage(GL_TEXTURE_2D, LEVEL, GL_RGBA, GL_UNSIGNED_BYTE,
                quarter.data());
  OkTexture::unbind();
  REQUIRE(OkTexture::alphaCoverage(alphasOf(quarter), OK_ALPHA_CUTOUT_THRESHOLD,
                                   1.0f) >= GRILLE_COVERAGE);
}
