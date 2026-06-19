#include "top_down_camera.hpp"

#include "../core/object.hpp"
#include "../math/math.hpp"

OkTopDownCamera::OkTopDownCamera(const std::string &name, int width, int height,
                                 float height_m, float fovDegrees)
    : OkCamera(name, width, height) {
  _height = height_m;
  _fov    = fovDegrees;
}

void OkTopDownCamera::updateForTarget(const OkObject *target, float dt) {
  (void)dt;
  if (target == nullptr) {
    return;
  }
  OkPoint p = target->getPosition();
  OkPoint eye(p.x(), p.y() + _height, p.z());
  OkPoint look(p.x(), p.y(), p.z());

  setPerspective(_fov, 1.0f, _height * 2.0f + 2000.0f);
  setPosition(eye);
  // Straight down, world north (+Z) up on screen.
  setRotation(OkMath::lookAt(eye, look, OkPoint(0.0f, 0.0f, 1.0f)));
  setSpeed(0.0f, 0.0f, 0.0f);
}
