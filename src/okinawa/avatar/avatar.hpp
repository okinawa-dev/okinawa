#ifndef OK_AVATAR_HPP
#define OK_AVATAR_HPP

#include "controller.hpp"

#include <vector>

class OkObject;
class OkCamera;
class OkInputState;
class OkCameraView;

/**
 * @brief A controllable representation of the player in a scene: a controlled
 *        object plus the input scheme (controller) that drives it. The camera
 *        rig (the views that observe the avatar) is added on top of this.
 *
 *        The avatar OWNS its controller (deleted in the destructor) but NOT the
 *        controlled object, which belongs to the scene. OkCore tracks the
 *        active avatar and updates it each frame; swapping the active avatar
 *        (on foot -> car) changes the controls in one step.
 */
class OkAvatar {
public:
  OkAvatar(OkObject *controlled, OkAvatarController *controller);
  ~OkAvatar();

  // Non-copyable (owns a controller).
  OkAvatar(const OkAvatar &)            = delete;
  OkAvatar &operator=(const OkAvatar &) = delete;

  // Drive the avatar for this frame from the input state. activeCamera is
  // passed through to the controller for camera-relative movement.
  void update(float dt, const OkInputState &input, OkCamera *activeCamera);

  OkObject           *getControlledObject() const { return _controlled; }
  OkAvatarController *getController() const { return _controller; }
  void                setController(OkAvatarController *controller);

  // Camera rig: views that observe this avatar. The avatar owns them. Register
  // each view's camera() with OkCore so the number keys can switch between
  // them. Every view is repositioned each frame in update().
  void          addView(OkCameraView *view);
  OkCameraView *viewForCamera(const OkCamera *camera) const;

private:
  OkObject                   *_controlled;  // not owned (the scene owns it)
  OkAvatarController         *_controller;  // owned
  std::vector<OkCameraView *> _views;       // owned
};

#endif  // OK_AVATAR_HPP
