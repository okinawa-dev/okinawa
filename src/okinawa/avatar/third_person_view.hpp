#ifndef OK_THIRD_PERSON_VIEW_HPP
#define OK_THIRD_PERSON_VIEW_HPP

#include "camera_view.hpp"

class OkCamera;
class OkObject;

/**
 * @brief Third-person camera: orbits behind/above the avatar and
 *        looks at it. The mouse orbits it (yaw freely, pitch clamped). Because
 *        it looks toward the avatar, the camera's forward (projected to the
 *        ground) is the direction the avatar walks "into the screen".
 */
class OkThirdPersonView : public OkCameraView {
public:
  OkThirdPersonView(OkCamera *camera, float distance = 12.0f,
                    float focusHeight = 1.5f);

  OkCamera *camera() const override { return _camera; }
  void      update(const OkObject &target, float dt) override;
  void      handleMouse(float yawDeg, float pitchDeg) override;

  void setDistance(float distance) { _distance = distance; }

private:
  OkCamera *_camera;
  float     _distance;
  float     _focusHeight;  // look this far above the avatar's origin
  float     _yaw;          // orbit angle around the avatar (radians)
  float     _pitch;        // orbit elevation (radians, clamped)
};

#endif  // OK_THIRD_PERSON_VIEW_HPP
