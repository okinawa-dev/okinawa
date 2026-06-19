#include "walk_controller.hpp"

#include "../core/camera.hpp"
#include "../core/object.hpp"
#include "../math/math.hpp"

OkWalkController::OkWalkController(float moveSpeed) {
  _moveSpeed = moveSpeed;
}

OkGroundMove OkWalkController::computeGroundMove(const OkInputState &input,
                                                const OkRotation  &cameraRotation,
                                                float speed,
                                                float dtSeconds) {
  const float eps = 1e-4f;

  // Camera forward/right projected onto the ground plane (drop Y) so movement
  // stays horizontal regardless of the camera's pitch.
  OkPoint forward = cameraRotation.getForwardVector();
  OkPoint right   = cameraRotation.getRightVector();
  OkPoint forwardGround(forward.x(), 0.0f, forward.z());
  OkPoint rightGround(right.x(), 0.0f, right.z());
  if (forwardGround.magnitude() > eps) forwardGround = forwardGround.normalize();
  if (rightGround.magnitude() > eps) rightGround = rightGround.normalize();

  OkPoint direction(0.0f, 0.0f, 0.0f);
  if (input.forward) direction += forwardGround;
  if (input.backward) direction -= forwardGround;
  if (input.strafeRight) direction += rightGround;
  if (input.strafeLeft) direction -= rightGround;

  OkGroundMove result;
  result.moved     = false;
  result.dx        = 0.0f;
  result.dz        = 0.0f;
  result.facingYaw = cameraRotation.getYaw();

  if (direction.magnitude() > eps) {
    direction       = direction.normalize();
    OkPoint step    = direction * (speed * dtSeconds);
    result.dx       = step.x();
    result.dz       = step.z();
    result.moved    = true;
    float pitch     = 0.0f;
    float yaw       = 0.0f;
    OkMath::directionVectorToAngles(direction, pitch, yaw);
    result.facingYaw = yaw;
  }

  return result;
}

void OkWalkController::update(float dt, const OkInputState &input,
                             OkObject &avatar, OkCamera *activeCamera) {
  // Use the camera's facing for movement; fall back to the avatar's own facing
  // when there is no active camera.
  OkRotation reference =
      activeCamera ? activeCamera->getRotation() : avatar.getRotation();

  OkGroundMove move =
      computeGroundMove(input, reference, _moveSpeed, dt / 1000.0f);
  if (move.moved) {
    avatar.move(move.dx, 0.0f, move.dz);
    avatar.setRotation(0.0f, move.facingYaw, 0.0f);
  }
}
