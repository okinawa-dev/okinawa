#include "avatar.hpp"

#include "../core/camera.hpp"

OkAvatar::OkAvatar(OkObject *controlled, OkAvatarController *controller) {
  _controlled = controlled;
  _controller = controller;
}

OkAvatar::~OkAvatar() {
  delete _controller;
  // _cameras are owned by OkCore; do not delete them here.
}

void OkAvatar::setController(OkAvatarController *controller) {
  if (_controller != controller) {
    delete _controller;
    _controller = controller;
  }
}

void OkAvatar::addCamera(OkCamera *camera) {
  if (camera) {
    _cameras.push_back(camera);
  }
}

void OkAvatar::update(float dt, const OkInputState &input) {
  if (_controller && _controlled) {
    _controller->update(dt, input, *_controlled);
  }
  for (std::size_t i = 0; i < _cameras.size(); i++) {
    // Skip cameras whose pose is frozen via MCP (set_camera_pose), so a
    // reproduced view is not overwritten by avatar tracking.
    if (!_cameras[i]->isPoseOverridden()) {
      _cameras[i]->updateForTarget(_controlled, dt);
    }
  }
}
