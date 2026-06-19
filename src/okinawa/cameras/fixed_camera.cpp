#include "fixed_camera.hpp"

#include "../core/object.hpp"
#include "../math/math.hpp"

OkFixedCamera::OkFixedCamera(const std::string &name, int width, int height)
    : OkCamera(name, width, height) {
  _position     = OkPoint(0.0f, 0.0f, 0.0f);
  _lookAtTarget = true;
}

void OkFixedCamera::place(const OkPoint &position, bool lookAtTarget) {
  _position     = position;
  _lookAtTarget = lookAtTarget;
  setPosition(_position);
}

void OkFixedCamera::updateForTarget(const OkObject *target, float dt) {
  (void)dt;
  setPosition(_position);
  if (_lookAtTarget && target != nullptr) {
    setRotation(OkMath::lookAt(_position, target->getPosition()));
  }
  setSpeed(0.0f, 0.0f, 0.0f);
}
