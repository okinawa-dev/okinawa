#include "top_down_view.hpp"

#include "../core/camera.hpp"
#include "../core/object.hpp"
#include "../math/math.hpp"

#include <algorithm>
#include <cmath>
#include <glm/trigonometric.hpp>

OkTopDownView::OkTopDownView(OkCamera *camera, float fovDegrees) {
  _camera  = camera;
  _fov     = fovDegrees;
  _centerX = 0.0f;
  _centerZ = 0.0f;
  _sizeX   = 0.0f;
  _sizeZ   = 0.0f;
  _groundY = 0.0f;
  _margin  = 1.1f;
}

void OkTopDownView::setBounds(float minX, float minZ, float maxX, float maxZ,
                             float groundY) {
  _centerX = (minX + maxX) * 0.5f;
  _centerZ = (minZ + maxZ) * 0.5f;
  _sizeX   = std::fabs(maxX - minX);
  _sizeZ   = std::fabs(maxZ - minZ);
  _groundY = groundY;
}

float OkTopDownView::computeHeight(float sizeX, float sizeZ, float fovDegrees,
                                   float margin) {
  float half     = std::max(sizeX, sizeZ) * 0.5f * margin;
  float halfFov  = glm::radians(fovDegrees) * 0.5f;
  float tanHalf  = std::tan(halfFov);
  if (tanHalf < 1e-5f) return half;  // degenerate fov guard
  return half / tanHalf;
}

void OkTopDownView::update(const OkObject &target, float dt) {
  (void)target;  // the top-down frames the sector, not the avatar
  (void)dt;
  if (!_camera) return;

  float   height = computeHeight(_sizeX, _sizeZ, _fov, _margin);
  OkPoint eye(_centerX, _groundY + height, _centerZ);
  OkPoint look(_centerX, _groundY, _centerZ);

  // Straight down, with world north (+Z) up on screen. Far plane clears the
  // height; near small.
  _camera->setPerspective(_fov, 1.0f, height * 2.0f + 1000.0f);
  _camera->setPosition(eye);
  _camera->setRotation(OkMath::lookAt(eye, look, OkPoint(0.0f, 0.0f, 1.0f)));
  _camera->setSpeed(0.0f, 0.0f, 0.0f);
}
