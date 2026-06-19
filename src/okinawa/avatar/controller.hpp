#ifndef OK_AVATAR_CONTROLLER_HPP
#define OK_AVATAR_CONTROLLER_HPP

class OkObject;
class OkCamera;
class OkInputState;

/**
 * @brief Input scheme for an avatar. An OkAvatar owns one controller; it
 *        interprets the per-frame input state and moves/acts on the controlled
 *        object. Different avatars (on foot, vehicle) use different controllers,
 *        which is how switching avatar swaps the controls.
 */
class OkAvatarController {
public:
  virtual ~OkAvatarController() {}

  /**
   * @brief Update the avatar for this frame.
   * @param dt           Delta time in milliseconds (engine loop units).
   * @param input        The current frame's input state.
   * @param avatar       The controlled object to move/orient.
   * @param activeCamera The active camera, so movement can be made
   *                     camera-relative. May be null.
   */
  virtual void update(float dt, const OkInputState &input, OkObject &avatar,
                      OkCamera *activeCamera) = 0;
};

#endif  // OK_AVATAR_CONTROLLER_HPP
