#include "okinawa/handlers/textures.hpp"
#include "okinawa/item/item.hpp"
#include "test-opengl.hpp"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

// An item's tint mask and alpha cutout, as state.
//
// What the shader does with them needs a frame to look at; what can be
// held to a test is the bookkeeping underneath: the flags start off, the
// mask is loaded through the texture cache like any other texture, it is
// shared rather than loaded twice, and it is released with the last item
// holding it. A mask leaked or freed early fails nothing on screen until
// another item asks for the same image.

namespace {

  // Any image the engine ships will do: the test is about who holds the
  // texture, not what is in it.
  const char *const MASK_PATH = "assets/project/okinawa_logo.png";

  // Whether the cache holds a texture under this name. Asked through
  // the list of names because getTexture() takes a reference of its
  // own, and a test that counts references cannot add one to look.
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

}  // namespace

TEST_CASE("An item starts with no tint mask and no cutout",
          "[item][tintmask]") {
  TestGLFWContext context;
  OkItem          item("plain");

  REQUIRE_FALSE(item.hasTintMask());
  REQUIRE(item.getTintMaskName().empty());
  REQUIRE_FALSE(item.getAlphaCutout());
  REQUIRE_FALSE(item.getMaterialLuminance(0));

  item.setAlphaCutout(true);
  REQUIRE(item.getAlphaCutout());
  item.setAlphaCutout(false);
  REQUIRE_FALSE(item.getAlphaCutout());

  item.setMaterialLuminance(1, true);
  REQUIRE(item.getMaterialLuminance(1));
  REQUIRE_FALSE(item.getMaterialLuminance(0));
  // A slot out of range is refused, not written past the end.
  item.setMaterialLuminance(3, true);
  REQUIRE_FALSE(item.getMaterialLuminance(3));
}

TEST_CASE("A tint mask that cannot be loaded leaves no mask",
          "[item][tintmask]") {
  TestGLFWContext context;
  OkItem          item("missing");

  item.setTintMask("tests/no-such-mask.png");
  REQUIRE_FALSE(item.hasTintMask());
  REQUIRE(item.getTintMaskName().empty());
}

TEST_CASE("A tint mask is shared through the texture cache and released",
          "[item][tintmask]") {
  TestGLFWContext context;
  REQUIRE_FALSE(isCached(MASK_PATH));

  {
    OkItem first("first");
    first.setTintMask(MASK_PATH);
    REQUIRE(first.hasTintMask());
    REQUIRE(first.getTintMaskName() == MASK_PATH);
    REQUIRE(isCached(MASK_PATH));

    {
      OkItem second("second");
      second.setTintMask(MASK_PATH);
      REQUIRE(second.hasTintMask());
    }
    // The second item gave its reference back and the first still
    // holds one, so the texture is still there: the two shared it.
    REQUIRE(isCached(MASK_PATH));

    // An empty path takes the mask off and gives the reference back.
    first.setTintMask("");
    REQUIRE_FALSE(first.hasTintMask());
    REQUIRE(first.getTintMaskName().empty());
    REQUIRE_FALSE(isCached(MASK_PATH));

    // Set again and left on: the item's destructor has to release it.
    first.setTintMask(MASK_PATH);
    REQUIRE(isCached(MASK_PATH));
  }
  REQUIRE_FALSE(isCached(MASK_PATH));
}
