#include "instanced_item.hpp"

#include "../config/config.hpp"
#include "../lighting/lighting.hpp"
#include "../math/frustum.hpp"
#include "../utils/logger.hpp"
#include "texture.hpp"
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec4.hpp>

// Per-instance attribute layout, starting after the mesh attributes
// (0 position, 1 uv, 2 normal): 3 = position+scale, 4 = orientation.
static const int OK_INST_ATTR_POS    = 3;
static const int OK_INST_ATTR_ORIENT = 4;
static const int OK_INST_FLOATS      = 8;
// The three zone colours an instance may carry, after those: one
// attribute a zone, three floats each.
static const int OK_INST_ATTR_TINT     = 5;
static const int OK_INST_TINT_FLOATS   = 9;
static const int OK_INST_LAYOUT_PLAIN  = 1;
static const int OK_INST_LAYOUT_TINTED = 2;

OkInstancedItem::OkInstancedItem(const std::string &name, float *vertexData,
                                 long vertexCount, unsigned int *indexData,
                                 long indexCount, int vertexStride)
    : OkItem(name, vertexData, vertexCount, indexData, indexCount,
             vertexStride) {
  _instanceVbo    = 0;
  _tintedCount    = 0;
  _bufferLayout   = 0;
  _drawnCount     = 0;
  _instanceCentre = {0.0f, 0.0f, 0.0f};
  _instanceRadius = 0.0f;
}

OkInstancedItem::OkInstancedItem(const std::string &name,
                                 const std::string &meshKey,
                                 const float *vertexData, long vertexCount,
                                 const unsigned int *indexData, long indexCount,
                                 int vertexStride)
    : OkItem(name, meshKey, vertexData, vertexCount, indexData, indexCount,
             vertexStride) {
  _instanceVbo    = 0;
  _tintedCount    = 0;
  _bufferLayout   = 0;
  _drawnCount     = 0;
  _instanceCentre = {0.0f, 0.0f, 0.0f};
  _instanceRadius = 0.0f;
}

OkInstancedItem::~OkInstancedItem() {
  if (_instanceVbo != 0) {
    glDeleteBuffers(1, &_instanceVbo);
    _instanceVbo = 0;
  }
}

void OkInstancedItem::growInstanceBounds(const Instance &inst) {
  // The mesh's own sphere, standing where this instance stands.
  float cx  = inst.x + sphereCenter[0] * inst.scale;
  float cy  = inst.y + sphereCenter[1] * inst.scale;
  float cz  = inst.z + sphereCenter[2] * inst.scale;
  float rad = radius * inst.scale;
  if (_instanceRadius <= 0.0f) {
    _instanceCentre = {cx, cy, cz};
    _instanceRadius = rad;
    return;
  }
  float dx   = cx - _instanceCentre[0];
  float dy   = cy - _instanceCentre[1];
  float dz   = cz - _instanceCentre[2];
  float away = std::sqrt(dx * dx + dy * dy + dz * dz);
  if (away + rad <= _instanceRadius) {
    return;
  }
  if (away + _instanceRadius <= rad) {
    _instanceCentre = {cx, cy, cz};
    _instanceRadius = rad;
    return;
  }
  float grown = (away + _instanceRadius + rad) * 0.5f;
  float shift = (grown - _instanceRadius) / (away > 1e-6f ? away : 1.0f);
  _instanceCentre[0] += dx * shift;
  _instanceCentre[1] += dy * shift;
  _instanceCentre[2] += dz * shift;
  _instanceRadius = grown;
}

int OkInstancedItem::addInstance(float x, float y, float z, float yaw,
                                 float scale) {
  Instance inst;
  inst.x       = x;
  inst.y       = y;
  inst.z       = z;
  inst.yaw     = yaw;
  inst.scale   = scale;
  inst.visible = true;
  _instances.push_back(inst);
  if (!_instanceTinted.empty()) {
    std::array<float, 9> none = {};
    _instanceTints.push_back(none);
    _instanceTinted.push_back(0);
  }
  growInstanceBounds(inst);
  return static_cast<int>(_instances.size()) - 1;
}

void OkInstancedItem::setInstance(int index, float x, float y, float z,
                                  float yaw, float scale) {
  if (index < 0 || index >= static_cast<int>(_instances.size())) {
    return;
  }
  _instances[static_cast<size_t>(index)].x     = x;
  _instances[static_cast<size_t>(index)].y     = y;
  _instances[static_cast<size_t>(index)].z     = z;
  _instances[static_cast<size_t>(index)].yaw   = yaw;
  _instances[static_cast<size_t>(index)].scale = scale;
}

void OkInstancedItem::setInstanceVisible(int index, bool visible) {
  if (index < 0 || index >= static_cast<int>(_instances.size())) {
    return;
  }
  _instances[static_cast<size_t>(index)].visible = visible;
}

void OkInstancedItem::clearInstances() {
  _instances.clear();
  _instanceTints.clear();
  _instanceTinted.clear();
  _tintedCount = 0;
}

void OkInstancedItem::setInstanceMaterialTint(int index, int slot, float r,
                                              float g, float b) {
  if (index < 0 || index >= static_cast<int>(_instances.size()) || slot < 0 ||
      slot >= MAT_SLOTS) {
    return;
  }
  size_t at = static_cast<size_t>(index);
  if (_instanceTinted.empty()) {
    std::array<float, 9> none = {};
    _instanceTints.assign(_instances.size(), none);
    _instanceTinted.assign(_instances.size(), 0);
  }
  if (_instanceTinted[at] == 0) {
    // From the item's colours, so the zones not named keep theirs.
    for (int k = 0; k < MAT_SLOTS; k++) {
      for (int c = 0; c < RGB; c++) {
        _instanceTints[at][(static_cast<size_t>(k) * RGB) +
                           static_cast<size_t>(c)] =
            material.slotTint[static_cast<size_t>(k)][static_cast<size_t>(c)];
      }
    }
    _instanceTinted[at] = 1;
    _tintedCount++;
  }
  size_t first                  = static_cast<size_t>(slot) * RGB;
  _instanceTints[at][first]     = r;
  _instanceTints[at][first + 1] = g;
  _instanceTints[at][first + 2] = b;
}

void OkInstancedItem::clearInstanceMaterialTints(int index) {
  if (index < 0 || index >= static_cast<int>(_instanceTinted.size())) {
    return;
  }
  size_t at = static_cast<size_t>(index);
  if (_instanceTinted[at] != 0) {
    _instanceTinted[at] = 0;
    _tintedCount--;
  }
}

/**
 * @brief Create the per-instance buffer and wire its attributes into the
 *        mesh VAO with an attribute divisor of 1 (advance once per
 *        instance instead of once per vertex).
 */
void OkInstancedItem::ensureInstanceBuffer(bool tinted) {
  int layout = tinted ? OK_INST_LAYOUT_TINTED : OK_INST_LAYOUT_PLAIN;
  if (_instanceVbo != 0 && _bufferLayout == layout) {
    return;
  }
  if (_instanceVbo == 0) {
    glGenBuffers(1, &_instanceVbo);
  }
  int     floats = OK_INST_FLOATS + (tinted ? OK_INST_TINT_FLOATS : 0);
  GLsizei stride = static_cast<GLsizei>(floats * sizeof(float));
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);

  // vec4: world position + uniform scale
  glVertexAttribPointer(OK_INST_ATTR_POS, 4, GL_FLOAT, GL_FALSE, stride,
                        nullptr);
  glEnableVertexAttribArray(OK_INST_ATTR_POS);
  glVertexAttribDivisor(OK_INST_ATTR_POS, 1);

  // vec4: cos(yaw), sin(yaw), spare, spare
  glVertexAttribPointer(OK_INST_ATTR_ORIENT, 4, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<GLvoid *>(4 * sizeof(float)));
  glEnableVertexAttribArray(OK_INST_ATTR_ORIENT);
  glVertexAttribDivisor(OK_INST_ATTR_ORIENT, 1);

  // vec3 a zone: the colours of the tint mask's three zones.
  for (int k = 0; k < MAT_SLOTS; k++) {
    GLuint attr = static_cast<GLuint>(OK_INST_ATTR_TINT + k);
    if (tinted) {
      size_t offset = (static_cast<size_t>(OK_INST_FLOATS) +
                       (static_cast<size_t>(k) * RGB)) *
                      sizeof(float);
      glVertexAttribPointer(attr, RGB, GL_FLOAT, GL_FALSE, stride,
                            reinterpret_cast<GLvoid *>(offset));
      glEnableVertexAttribArray(attr);
      glVertexAttribDivisor(attr, 1);
    } else {
      glDisableVertexAttribArray(attr);
    }
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  _bufferLayout = layout;
}

/**
 * @brief Draw every visible instance in ONE call. Instances are frustum
 *        culled individually (mesh bounding sphere at the instance
 *        position), so a scene full of lamps only uploads what is on
 *        screen.
 */
void OkInstancedItem::drawSelf() {
  if (!visible || _instances.empty()) {
    _drawnCount = 0;
    return;
  }

  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  if (currentProgram == 0) {
    return;
  }

  // Collect the visible, on-screen instances.
  //
  // An instance's position is measured INSIDE this item, so the test
  // has to be made where the instance actually stands: through the
  // item's own transform. Testing the raw numbers means testing a point
  // near the origin of the world, and every instance of a set that hangs
  // off something -- a district, a room -- is culled at once. The city
  // lost every one of its windows to exactly that.
  glm::mat4        model   = getTransformMatrix();
  const OkFrustum *frustum = OkFrustum::getActive();
  bool             tinted  = _tintedCount > 0;
  int              floats = OK_INST_FLOATS + (tinted ? OK_INST_TINT_FLOATS : 0);
  _uploadScratch.clear();
  _uploadScratch.reserve(_instances.size() * static_cast<size_t>(floats));
  for (size_t i = 0; i < _instances.size(); i++) {
    const Instance &inst = _instances[i];
    if (!inst.visible) {
      continue;
    }
    if (frustum != nullptr) {
      glm::vec4 centre =
          model * glm::vec4(inst.x + sphereCenter[0] * inst.scale,
                            inst.y + sphereCenter[1] * inst.scale,
                            inst.z + sphereCenter[2] * inst.scale, 1.0f);
      float cx = centre.x;
      float cy = centre.y;
      float cz = centre.z;
      if (OkFrustum::isBeyondDrawDistance(cx, cy, cz, radius * inst.scale) ||
          !frustum->containsSphere(cx, cy, cz, radius * inst.scale)) {
        OkFrustum::addCulled();
        continue;
      }
    }
    _uploadScratch.push_back(inst.x);
    _uploadScratch.push_back(inst.y);
    _uploadScratch.push_back(inst.z);
    _uploadScratch.push_back(inst.scale);
    _uploadScratch.push_back(std::cos(inst.yaw));
    _uploadScratch.push_back(std::sin(inst.yaw));
    _uploadScratch.push_back(0.0f);
    _uploadScratch.push_back(0.0f);
    if (tinted) {
      // Its own colours, or the item's for one that has none.
      for (int k = 0; k < MAT_SLOTS; k++) {
        for (int c = 0; c < RGB; c++) {
          size_t q = (static_cast<size_t>(k) * RGB) + static_cast<size_t>(c);
          _uploadScratch.push_back(_instanceTinted[i] != 0
                                       ? _instanceTints[i][q]
                                       : material.slotTint[static_cast<size_t>(
                                             k)][static_cast<size_t>(c)]);
        }
      }
    }
  }
  _drawnCount =
      static_cast<int>(_uploadScratch.size() / static_cast<size_t>(floats));
  if (_drawnCount == 0) {
    return;
  }

  ensureInstanceBuffer(tinted);
  glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(_uploadScratch.size() * sizeof(float)),
               _uploadScratch.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Where this ITEM stands: the instances are placed within it, so an
  // instanced item hangs off a parent and moves with it like anything
  // else. It used to pass the identity, which pinned every instance to
  // the origin of the world and made a parent above it mean nothing.
  GLint modelLoc = glGetUniformLocation(currentProgram, "model");
  if (modelLoc != -1) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  }
  GLint instLoc = glGetUniformLocation(currentProgram, "instanced");
  if (instLoc != -1) {
    glUniform1i(instLoc, 1);
  }
  GLint instTintLoc = glGetUniformLocation(currentProgram, "instanceTints");
  if (instTintLoc != -1) {
    glUniform1f(instTintLoc, tinted ? 1.0f : 0.0f);
  }
  bool drawTexture =
      OkConfig::getBool("graphics.textures") && texture && texture->isLoaded();
  GLint hasTexLoc = glGetUniformLocation(currentProgram, "hasTexture");
  // The mask and the material tints, the same ones an ordinary draw
  // sends. Without them the instances came out wearing whatever the
  // last item drawn had left in those uniforms -- which looked almost
  // right, and made a hundred thousand windows take their glass colour
  // from whichever building happened to be drawn before them.
  // And the same blending and lighting: every instance wears the item's
  // one material.
  _beginMaterial(static_cast<unsigned int>(currentProgram), material);
  // The cross-fade uniform has to be written even when this item is not
  // fading. It is program state, not object state: leaving it alone
  // means inheriting whatever the last object drawn happened to set,
  // so a solid object drawn after a dissolving one comes out dithered.
  {
    GLint fadeLoc = glGetUniformLocation(currentProgram, "itemFade");
    if (fadeLoc != -1) {
      glUniform1f(fadeLoc, fade);
    }
    GLint fadeInvLoc = glGetUniformLocation(currentProgram, "itemFadeInvert");
    if (fadeInvLoc != -1) {
      glUniform1f(fadeInvLoc, fadeInverted ? 1.0f : 0.0f);
    }
  }
  if (drawTexture) {
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GLint texLoc = glGetUniformLocation(currentProgram, "texture0");
    if (texLoc != -1) {
      glUniform1i(texLoc, 0);
    }
    if (hasTexLoc != -1) {
      glUniform1i(hasTexLoc, 1);
    }
  } else {
    if (hasTexLoc != -1) {
      glUniform1i(hasTexLoc, 0);
    }
    GLint colorLoc = glGetUniformLocation(currentProgram, "wireframeColor");
    if (colorLoc != -1) {
      glUniform4f(colorLoc, fillColor[0], fillColor[1], fillColor[2],
                  fillColor[3]);
    }
  }

  glBindVertexArray(VAO);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawElementsInstanced(drawMode, static_cast<GLsizei>(numIndices),
                          GL_UNSIGNED_INT, nullptr,
                          static_cast<GLsizei>(_drawnCount));
  OkFrustum::addDraw((numIndices / 3) * _drawnCount);

  // Wireframe overlay, on the same instanced draw. One pass covers
  // every instance: without it a whole class of objects -- the ones
  // drawn in bulk, which is where a mesh problem is most likely to
  // repeat -- silently stays solid while the rest of the scene shows
  // its triangles. Unlit, like the plain item's overlay.
  bool wire = drawWireframe ||
              (wireframeGlobal && OkConfig::getBool("graphics.wireframe"));
  if (wire) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    GLint wLitLoc = glGetUniformLocation(currentProgram, "lightingOn");
    if (wLitLoc != -1) {
      glUniform1f(wLitLoc, 0.0f);
    }
    if (hasTexLoc != -1) {
      glUniform1i(hasTexLoc, 0);
    }
    GLint colorLoc = glGetUniformLocation(currentProgram, "wireframeColor");
    if (colorLoc != -1) {
      glUniform4f(colorLoc, wireframeColor[0], wireframeColor[1],
                  wireframeColor[2], 1.0f);
    }
    glDrawElementsInstanced(drawMode, static_cast<GLsizei>(numIndices),
                            GL_UNSIGNED_INT, nullptr,
                            static_cast<GLsizei>(_drawnCount));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (wLitLoc != -1) {
      glUniform1f(wLitLoc, 1.0f);
    }
  }
  glBindVertexArray(0);

  if (texture) {
    OkTexture::unbind();
  }
  if (instLoc != -1) {
    glUniform1i(instLoc, 0);
  }
  if (instTintLoc != -1) {
    glUniform1f(instTintLoc, 0.0f);
  }
  _endMaterial(static_cast<unsigned int>(currentProgram), material);
}
