#include "okinawa/config/config.hpp"
#include "okinawa/handlers/textures.hpp"
#include "okinawa/item/group.hpp"
#include "okinawa/item/instanced_item.hpp"
#include "okinawa/item/item.hpp"
#include "okinawa/item/material.hpp"
#include "okinawa/render/render_target.hpp"
#include "okinawa/shaders/shaders.hpp"
#include "okinawa/utils/assets.hpp"
#include "test-opengl.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

// What the materials look like, read back from the pixels.
//
// The other tests of an item's materials hold its state. These draw: the
// engine's own world shaders, an offscreen target a few pixels across, a
// quad or two, and the colour that came out. A blend mode that is set
// and stored and then drawn in the wrong pass passes every test of
// state; it does not pass these.

namespace {

  // The target's side, in pixels, and where the colour is read: the
  // middle, and the middle of each half for the instances.
  const int SIDE  = 8;
  const int MID   = 4;
  const int LEFT  = 2;
  const int RIGHT = 6;

  // How far a channel may be from what is expected: the target holds
  // eight bits a channel.
  const float CLOSE = 0.02f;

  // Where the near and the far quad stand, in front of an eye at the
  // origin looking down -z.
  const float NEAR_Z = -1.0f;
  const float FAR_Z  = -5.0f;

  /** @brief The world program, a target and the state a pass needs. */
  class Stage {
  public:
    Stage() {
      program = OkShader::createProgram(
          OkAssets::loadShaderSource("world.vert.glsl"),
          OkAssets::loadShaderSource("world.frag.glsl"));
      target.resize(SIDE, SIDE);
    }
    ~Stage() {
      if (program != 0) {
        glDeleteProgram(program);
      }
    }

    void begin() {
      target.bind();
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glDepthMask(GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDisable(GL_BLEND);
      glDisable(GL_CULL_FACE);
      glUseProgram(program);
      // The engine's settings are one for the whole run, and another test
      // may have left them changed: a wireframe overlay draws the quad's
      // diagonal straight through the pixel read here.
      OkConfig::setBool("graphics.wireframe", false);
      OkConfig::setBool("graphics.textures", true);
      glm::mat4 view = glm::mat4(1.0f);
      glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
      glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE,
                         glm::value_ptr(view));
      OkItem::setPassView(glm::value_ptr(view));
      glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1,
                         GL_FALSE, glm::value_ptr(proj));
      // No light, no air: the colour out is the colour in.
      glUniform1f(glGetUniformLocation(program, "lightingOn"), 0.0f);
      glUniform3f(glGetUniformLocation(program, "sceneTint"), 1.0f, 1.0f, 1.0f);
      glUniform1f(glGetUniformLocation(program, "fogDensity"), 0.0f);
      glUniform1f(glGetUniformLocation(program, "shadowStrength"), 0.0f);
      glUniform1i(glGetUniformLocation(program, "shadowCascades"), 0);
      glUniform1f(glGetUniformLocation(program, "pointLightLevel"), 0.0f);
      glUniform1i(glGetUniformLocation(program, "pointLightCount"), 0);
      glUniform1f(glGetUniformLocation(program, "clusteredOn"), 0.0f);
      glUniform1f(glGetUniformLocation(program, "itemFade"), 1.0f);
      // Every sampler on a unit of its own: two of different kinds on
      // one unit fail the draw.
      glUniform1i(glGetUniformLocation(program, "texture0"), 0);
      glUniform1i(glGetUniformLocation(program, "tintMask"), 1);
      glUniform1i(glGetUniformLocation(program, "shadowMap"), 3);
      glUniform1i(glGetUniformLocation(program, "clusterLights"), 4);
      glUniform1i(glGetUniformLocation(program, "clusterIndices"), 5);
      glUniform1i(glGetUniformLocation(program, "clusterGrid"), 6);
    }

    /** @brief Both passes over the objects, as a scene draws them. */
    static void draw(const std::vector<OkObject *> &objects) {
      for (size_t i = 0; i < objects.size(); i++) {
        objects[i]->drawPass(false);
      }
      for (size_t i = 0; i < objects.size(); i++) {
        objects[i]->drawPass(true);
      }
    }

    static std::array<float, 4> pixel(int x, int y) {
      std::array<float, 4> out = {0.0f, 0.0f, 0.0f, 0.0f};
      glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, out.data());
      return out;
    }

    void end() {
      target.unbind();
    }

    GLuint         program;
    OkRenderTarget target;
  };

  /** @brief A quad across the whole view at depth z, as vertices. */
  std::array<float, 20> quadAt(float z, float half) {
    std::array<float, 20> quad = {
        -half, -half, z, 0.0f, 0.0f,  //
        half,  -half, z, 1.0f, 0.0f,  //
        half,  half,  z, 1.0f, 1.0f,  //
        -half, half,  z, 0.0f, 1.0f,  //
    };
    return quad;
  }

  const std::array<unsigned int, 6> QUAD_FACES = {0, 1, 2, 0, 2, 3};

  /** @brief An untextured item of one quad, in a colour. */
  OkItem *quadItem(const std::string &name, float z, float r, float g,
                   float b) {
    std::array<float, 20>       quad  = quadAt(z, 1.0f);
    std::array<unsigned int, 6> faces = QUAD_FACES;
    OkItem *item = new OkItem(name, quad.data(), static_cast<long>(quad.size()),
                              faces.data(), static_cast<long>(faces.size()));
    item->setFillColor(r, g, b);
    return item;
  }

  bool close(float a, float b) {
    return std::fabs(a - b) <= CLOSE;
  }

}  // namespace

TEST_CASE("A surface seen through is mixed with the solid one behind it",
          "[item][render]") {
  TestGLFWContext context;
  Stage           stage;
  REQUIRE(stage.program != 0);
  REQUIRE(stage.target.isValid());

  // The pane first in the list and the wall after it: drawn in that
  // order the wall would paint over the pane.
  OkItem *pane = quadItem("pane", NEAR_Z, 0.0f, 0.0f, 1.0f);
  pane->setBlendMode(OK_BLEND_ALPHA);
  pane->setOpacity(0.5f);
  OkItem *wall = quadItem("wall", FAR_Z, 1.0f, 0.0f, 0.0f);

  stage.begin();
  Stage::draw({pane, wall});
  std::array<float, 4> px = Stage::pixel(MID, MID);
  stage.end();
  CAPTURE(px[0], px[1], px[2]);

  REQUIRE(close(px[0], 0.5f));
  REQUIRE(close(px[1], 0.0f));
  REQUIRE(close(px[2], 0.5f));

  delete pane;
  delete wall;
}

TEST_CASE("One item draws its solid range and its range seen through",
          "[item][render]") {
  TestGLFWContext context;
  Stage           stage;

  // The pane's range is added first, and is the nearer.
  std::array<float, 20> pane  = quadAt(NEAR_Z, 1.0f);
  std::array<float, 20> frame = quadAt(FAR_Z, 1.0f);
  OkItem                window("window");
  window.addMesh(pane.data(), static_cast<long>(pane.size()), QUAD_FACES.data(),
                 static_cast<long>(QUAD_FACES.size()), "");
  window.addMesh(frame.data(), static_cast<long>(frame.size()),
                 QUAD_FACES.data(), static_cast<long>(QUAD_FACES.size()), "");
  window.upload();
  window.setFillColor(1.0f, 1.0f, 1.0f);

  OkMaterial glass;
  glass.blend = OK_BLEND_ALPHA;
  glass.tint  = {0.0f, 1.0f, 0.0f, 0.5f};
  REQUIRE(window.setRangeMaterial(0, glass));
  OkMaterial wood;
  wood.tint = {1.0f, 0.0f, 0.0f, 1.0f};
  REQUIRE(window.setRangeMaterial(1, wood));

  stage.begin();
  Stage::draw({&window});
  std::array<float, 4> px = Stage::pixel(MID, MID);
  stage.end();
  CAPTURE(px[0], px[1], px[2]);

  // Green glass, half there, over the red frame.
  REQUIRE(close(px[0], 0.5f));
  REQUIRE(close(px[1], 0.5f));
  REQUIRE(close(px[2], 0.0f));
}

TEST_CASE("Ranges seen through are drawn furthest first", "[item][render]") {
  TestGLFWContext context;
  Stage           stage;

  // The near pane's range is added first: in that order the far one
  // would be mixed in over it.
  std::array<float, 20> nearPane = quadAt(NEAR_Z, 1.0f);
  std::array<float, 20> farPane  = quadAt(FAR_Z, 1.0f);
  OkItem                panes("panes");
  panes.addMesh(nearPane.data(), static_cast<long>(nearPane.size()),
                QUAD_FACES.data(), static_cast<long>(QUAD_FACES.size()), "");
  panes.addMesh(farPane.data(), static_cast<long>(farPane.size()),
                QUAD_FACES.data(), static_cast<long>(QUAD_FACES.size()), "");
  panes.upload();
  panes.setFillColor(1.0f, 1.0f, 1.0f);

  OkMaterial red;
  red.blend = OK_BLEND_ALPHA;
  red.tint  = {1.0f, 0.0f, 0.0f, 0.5f};
  OkMaterial green;
  green.blend = OK_BLEND_ALPHA;
  green.tint  = {0.0f, 1.0f, 0.0f, 0.5f};
  REQUIRE(panes.setRangeMaterial(0, red));
  REQUIRE(panes.setRangeMaterial(1, green));

  stage.begin();
  Stage::draw({&panes});
  std::array<float, 4> px = Stage::pixel(MID, MID);
  stage.end();
  CAPTURE(px[0], px[1], px[2]);

  // Far green over black is (0, .5, 0); near red over that is
  // (.5, .25, 0). The other order gives (.25, .5, 0).
  REQUIRE(close(px[0], 0.5f));
  REQUIRE(close(px[1], 0.25f));
  REQUIRE(close(px[2], 0.0f));
}

TEST_CASE("A group draws each member in its own pass", "[item][render]") {
  TestGLFWContext context;
  Stage           stage;

  OkItem *pane = quadItem("pane", NEAR_Z, 0.0f, 0.0f, 1.0f);
  pane->setBlendMode(OK_BLEND_ALPHA);
  pane->setOpacity(0.5f);
  OkItem *wall = quadItem("wall", FAR_Z, 1.0f, 0.0f, 0.0f);

  OkItemGroup group("group");
  group.addItem(pane);
  group.addItem(wall);

  stage.begin();
  Stage::draw({&group});
  std::array<float, 4> px = Stage::pixel(MID, MID);
  stage.end();
  CAPTURE(px[0], px[1], px[2]);

  REQUIRE(close(px[0], 0.5f));
  REQUIRE(close(px[1], 0.0f));
  REQUIRE(close(px[2], 0.5f));

  delete pane;
  delete wall;
}

TEST_CASE("Each instance wears its own zone colours", "[item][render]") {
  TestGLFWContext context;
  Stage           stage;

  // A white texture, and a mask that is all zone 0: the colour drawn is
  // zone 0's tint.
  const std::array<unsigned char, 4> white = {255, 255, 255, 255};
  const std::array<unsigned char, 4> zone0 = {255, 0, 0, 255};
  OkTexture                         *texture =
      OkTextureHandler::getInstance()->createTextureFromRawData(
          "render-test-white", white.data(), 1, 1, 4);
  OkTexture *mask = OkTextureHandler::getInstance()->createTextureFromRawData(
      "render-test-zone0", zone0.data(), 1, 1, 4);
  REQUIRE(texture != nullptr);
  REQUIRE(mask != nullptr);

  std::array<float, 20>       quad  = quadAt(NEAR_Z, 0.5f);
  std::array<unsigned int, 6> faces = QUAD_FACES;
  {
    OkInstancedItem items("instances", quad.data(),
                          static_cast<long>(quad.size()), faces.data(),
                          static_cast<long>(faces.size()));
    items.setTexture("render-test-white", texture);
    items.setTintMask("render-test-zone0");
    items.setMaterialTint(0, 0.0f, 0.0f, 1.0f);
    int left  = items.addInstance(-0.5f, 0.0f, 0.0f);
    int right = items.addInstance(0.5f, 0.0f, 0.0f);
    REQUIRE_FALSE(items.hasInstanceTints());

    // Alike to begin with: both the item's blue.
    stage.begin();
    Stage::draw({&items});
    std::array<float, 4> a = Stage::pixel(LEFT, MID);
    std::array<float, 4> b = Stage::pixel(RIGHT, MID);
    stage.end();
    REQUIRE(close(a[2], 1.0f));
    REQUIRE(close(b[2], 1.0f));

    // Then one of them red: the other keeps the item's colour.
    items.setInstanceMaterialTint(left, 0, 1.0f, 0.0f, 0.0f);
    REQUIRE(items.hasInstanceTints());
    stage.begin();
    Stage::draw({&items});
    a = Stage::pixel(LEFT, MID);
    b = Stage::pixel(RIGHT, MID);
    stage.end();
    REQUIRE(close(a[0], 1.0f));
    REQUIRE(close(a[2], 0.0f));
    REQUIRE(close(b[0], 0.0f));
    REQUIRE(close(b[2], 1.0f));

    // And back.
    items.clearInstanceMaterialTints(left);
    REQUIRE_FALSE(items.hasInstanceTints());
    stage.begin();
    Stage::draw({&items});
    a = Stage::pixel(LEFT, MID);
    stage.end();
    REQUIRE(close(a[2], 1.0f));
    (void)right;
  }
  OkTextureHandler::getInstance()->removeReference("render-test-white");
  OkTextureHandler::getInstance()->removeReference("render-test-zone0");
}
