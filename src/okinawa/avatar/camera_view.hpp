#ifndef OK_CAMERA_VIEW_HPP
#define OK_CAMERA_VIEW_HPP

class OkCamera;
class OkObject;

/**
 * @brief A camera bound to an avatar: a behaviour that repositions its camera
 *        relative to the avatar every frame. Concrete views: third-person
 *        follow, top-down. An avatar owns its views; OkCore's number-key
 *        switching selects which view's camera is rendered.
 */
class OkCameraView {
public:
  virtual ~OkCameraView() {}

  // The camera this view drives (registered with OkCore for switching).
  virtual OkCamera *camera() const = 0;

  // Reposition the camera for this frame, given the avatar it observes.
  virtual void update(const OkObject &target, float dt) = 0;

  // Mouse delta in degrees (already sensitivity-scaled). Default: ignore (e.g.
  // the top-down view). Third-person consumes it as orbit.
  virtual void handleMouse(float yawDeg, float pitchDeg) {
    (void)yawDeg;
    (void)pitchDeg;
  }
};

#endif  // OK_CAMERA_VIEW_HPP
