#include "third_person_camera.hpp"

#include "../core/object.hpp"
#include "../math/math.hpp"

#include <algorithm>
#include <cmath>
#include <glm/trigonometric.hpp>

OkThirdPersonCamera::OkThirdPersonCamera(const std::string &name, int width,
                                         int height, float distance,
                                         float focusHeight)
    : OkCamera(name, width, height) {
  _distance    = distance;
  _focusHeight = focusHeight;
  _yaw         = 0.0f;
  _pitch       = glm::radians(20.0f);
}

void OkThirdPersonCamera::look(float yawDeg, float pitchDeg) {
  _yaw -= glm::radians(yawDeg);
  _pitch += glm::radians(pitchDeg);
  const float minPitch = glm::radians(-5.0f);
  const float maxPitch = glm::radians(75.0f);
  _pitch               = std::max(minPitch, std::min(maxPitch, _pitch));
}

void OkThirdPersonCamera::updateForTarget(const OkObject *target, float dt) {
  (void)dt;
  if (target == nullptr) {
    return;
  }
  OkPoint focus = target->getPosition() + OkPoint(0.0f, _focusHeight, 0.0f);
  float   cp    = std::cos(_pitch);
  float   sp    = std::sin(_pitch);
  OkPoint back(std::sin(_yaw) * cp, sp, std::cos(_yaw) * cp);
  OkPoint eye = focus + back * _distance;

  setPosition(eye);
  setRotation(OkMath::lookAt(eye, focus));  // aim at the target
  setSpeed(0.0f, 0.0f, 0.0f);
}
