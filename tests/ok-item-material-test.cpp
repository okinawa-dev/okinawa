#include "okinawa/handlers/textures.hpp"
#include "okinawa/item/item.hpp"
#include "okinawa/item/material.hpp"
#include "test-opengl.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

// An item's materials, as state.
//
// What a blend mode looks like needs a frame; what can be held to a test
// is which pass each range is drawn in, that a range falls back on the
// item's material until it is given one, and that a tint mask named by a
// range's material is counted in the texture cache and given back.

namespace {

  const char *const MASK_PATH = "assets/project/okinawa_logo.png";

  bool isCached(const std::string &path) {
    std::vector<std::string> names =
        OkTextureHandler::getInstance()->getTextureNames();
    for (size_t i = 0; i < names.size(); i++) {
      if (names[i] == path) {
        return true;
      }
    }
    return false;
  }

  // Two quads in one item, each a range of its own.
  void twoQuads(OkItem *item) {
    const std::array<float, 20> quad = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  //
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  //
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  //
    };
    const std::array<unsigned int, 6> faces = {0, 1, 2, 0, 2, 3};
    item->addMesh(quad.data(), static_cast<long>(quad.size()), faces.data(),
                  static_cast<long>(faces.size()), "");
    item->addMesh(quad.data(), static_cast<long>(quad.size()), faces.data(),
                  static_cast<long>(faces.size()), "");
    item->upload();
  }

}  // namespace

TEST_CASE("A material starts solid, lit and untinted", "[item][material]") {
  OkMaterial mat;
  REQUIRE(mat.blend == OK_BLEND_OPAQUE);
  REQUIRE_FALSE(mat.unlit);
  REQUIRE_FALSE(mat.isBlended());
  REQUIRE(mat.tint[3] == 1.0f);
  REQUIRE(mat.tintMask == nullptr);

  mat.blend = OK_BLEND_CUTOUT;
  REQUIRE_FALSE(mat.isBlended());
  mat.blend = OK_BLEND_ALPHA;
  REQUIRE(mat.isBlended());
  mat.blend = OK_BLEND_ADDITIVE;
  REQUIRE(mat.isBlended());
}

TEST_CASE("The old switches are the blend mode", "[item][material]") {
  TestGLFWContext context;
  OkItem          item("switches");

  REQUIRE(item.getBlendMode() == OK_BLEND_OPAQUE);
  item.setAdditive(true);
  REQUIRE(item.getBlendMode() == OK_BLEND_ADDITIVE);
  REQUIRE(item.isBlended());
  // Turning off a switch that is not the one on leaves the mode alone.
  item.setAlphaCutout(false);
  REQUIRE(item.getBlendMode() == OK_BLEND_ADDITIVE);
  item.setAdditive(false);
  REQUIRE(item.getBlendMode() == OK_BLEND_OPAQUE);
  item.setAlphaCutout(true);
  REQUIRE(item.getBlendMode() == OK_BLEND_CUTOUT);
  REQUIRE(item.getAlphaCutout());
  REQUIRE_FALSE(item.isBlended());

  item.setOpacity(0.25f);
  REQUIRE(item.getOpacity() == 0.25f);
  item.setOpacity(7.0f);
  REQUIRE(item.getOpacity() == 1.0f);
}

TEST_CASE("An item is drawn in the pass of its material", "[item][material]") {
  TestGLFWContext context;
  OkItem          item("whole");

  REQUIRE(item.drawsInPass(false));
  REQUIRE_FALSE(item.drawsInPass(true));
  item.setBlendMode(OK_BLEND_ALPHA);
  REQUIRE_FALSE(item.drawsInPass(false));
  REQUIRE(item.drawsInPass(true));
}

TEST_CASE("A range wears the item's material until it has its own",
          "[item][material]") {
  TestGLFWContext context;
  OkItem          item("frame-and-pane");
  twoQuads(&item);
  REQUIRE(item.getMaterialCount() == 2);

  item.setTintColor(0.5f, 0.5f, 0.5f, 1.0f);
  REQUIRE(item.getRangeMaterial(1)->tint[0] == 0.5f);
  REQUIRE(item.getRangeMaterial(2) == nullptr);

  OkMaterial glass;
  glass.blend   = OK_BLEND_ALPHA;
  glass.tint[3] = 0.4f;
  REQUIRE(item.setRangeMaterial(1, glass));
  REQUIRE_FALSE(item.setRangeMaterial(2, glass));

  // The pane is seen through, the frame is solid: one item, both passes.
  REQUIRE(item.getRangeMaterial(0)->blend == OK_BLEND_OPAQUE);
  REQUIRE(item.getRangeMaterial(1)->blend == OK_BLEND_ALPHA);
  REQUIRE(item.getRangeMaterial(1)->tint[3] == 0.4f);
  REQUIRE(item.drawsInPass(false));
  REQUIRE(item.drawsInPass(true));
  // The item as a whole is still what its own material says.
  REQUIRE_FALSE(item.isBlended());

  // Into a shadow map go the solid ranges, once.
  OkItem::setShadowPass(true);
  REQUIRE(item.drawsInPass(false));
  REQUIRE_FALSE(item.drawsInPass(true));
  OkItem::setShadowPass(false);

  // The item's material no longer reaches the range that has its own.
  item.setTintColor(0.1f, 0.1f, 0.1f, 1.0f);
  REQUIRE(item.getRangeMaterial(0)->tint[0] == 0.1f);
  REQUIRE(item.getRangeMaterial(1)->tint[0] == 1.0f);

  item.clearRangeMaterial(1);
  REQUIRE(item.getRangeMaterial(1)->blend == OK_BLEND_OPAQUE);
  REQUIRE_FALSE(item.drawsInPass(true));
}

TEST_CASE("A range's tint mask is counted and given back", "[item][material]") {
  TestGLFWContext context;
  REQUIRE_FALSE(isCached(MASK_PATH));
  {
    OkItem item("masked-range");
    twoQuads(&item);

    OkMaterial tinted;
    tinted.tintMaskName = MASK_PATH;
    REQUIRE(item.setRangeMaterial(0, tinted));
    REQUIRE(isCached(MASK_PATH));
    REQUIRE(item.getRangeMaterial(0)->tintMask != nullptr);
    // The item's own material has none: the mask is the range's.
    REQUIRE_FALSE(item.hasTintMask());

    // Replacing the material gives the mask back.
    REQUIRE(item.setRangeMaterial(0, OkMaterial()));
    REQUIRE_FALSE(isCached(MASK_PATH));

    REQUIRE(item.setRangeMaterial(1, tinted));
    REQUIRE(isCached(MASK_PATH));
  }
  // And so does the item going away.
  REQUIRE_FALSE(isCached(MASK_PATH));
}
