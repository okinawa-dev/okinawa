#ifndef OK_WALK_CONTROLLER_HPP
#define OK_WALK_CONTROLLER_HPP

#include "../input/input.hpp"
#include "../math/rotation.hpp"
#include "controller.hpp"

/**
 * @brief Result of a ground-movement step: the XZ delta to apply and the yaw
 *        the avatar should face (toward its movement). moved is false when
 *        there is no net input.
 */
struct OkGroundMove {
  bool  moved;
  float dx;
  float dz;
  float facingYaw;
};

/**
 * @brief Stock controller: camera-relative movement on the ground plane
 *        (GTA-style). W/S move along the camera's forward (projected onto the
 *        ground), A/D strafe along its right; the avatar turns to face its
 *        movement direction. Y is left untouched (no terrain-follow yet).
 */
class OkWalkController : public OkAvatarController {
public:
  explicit OkWalkController(float moveSpeed = 5.0f);

  void update(float dt, const OkInputState &input, OkObject &avatar,
              OkCamera *activeCamera) override;

  void  setMoveSpeed(float speed) { _moveSpeed = speed; }
  float getMoveSpeed() const { return _moveSpeed; }

  /**
   * @brief Pure movement maths, factored out so it can be unit-tested without
   *        a window/GL. Projects the camera's forward/right onto the ground,
   *        composes the input into a normalised direction, and returns the
   *        scaled XZ delta plus the facing yaw.
   * @param dtSeconds Delta time in seconds.
   */
  static OkGroundMove computeGroundMove(const OkInputState &input,
                                        const OkRotation  &cameraRotation,
                                        float speed, float dtSeconds);

private:
  float _moveSpeed;  // units per second
};

#endif  // OK_WALK_CONTROLLER_HPP
