#include "third_person_view.hpp"

#include "../core/camera.hpp"
#include "../core/object.hpp"
#include "../math/math.hpp"

#include <algorithm>
#include <cmath>
#include <glm/trigonometric.hpp>

OkThirdPersonView::OkThirdPersonView(OkCamera *camera, float distance,
                                     float focusHeight) {
  _camera      = camera;
  _distance    = distance;
  _focusHeight = focusHeight;
  _yaw         = 0.0f;
  _pitch       = glm::radians(20.0f);  // start slightly above the avatar
}

void OkThirdPersonView::handleMouse(float yawDeg, float pitchDeg) {
  // Match the free-fly feel: mouse-x yaws, mouse-y pitches.
  _yaw -= glm::radians(yawDeg);
  _pitch += glm::radians(pitchDeg);

  // Clamp pitch so the camera stays above the ground and below straight-down.
  const float minPitch = glm::radians(-5.0f);
  const float maxPitch = glm::radians(75.0f);
  _pitch               = std::max(minPitch, std::min(maxPitch, _pitch));
}

void OkThirdPersonView::update(const OkObject &target, float dt) {
  (void)dt;
  if (!_camera) return;

  OkPoint focus = target.getPosition() + OkPoint(0.0f, _focusHeight, 0.0f);

  // Unit vector from the focus toward the camera (behind), from the orbit
  // angles; then place the camera _distance away and look back at the focus.
  float   cp = std::cos(_pitch);
  float   sp = std::sin(_pitch);
  OkPoint back(std::sin(_yaw) * cp, sp, std::cos(_yaw) * cp);
  OkPoint eye = focus + back * _distance;

  _camera->setPosition(eye);
  _camera->setRotation(OkMath::lookAt(eye, focus));
  _camera->setSpeed(0.0f, 0.0f, 0.0f);  // positioned directly, not integrated
}
