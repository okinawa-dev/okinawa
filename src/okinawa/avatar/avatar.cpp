#include "avatar.hpp"

#include "../core/camera.hpp"
#include "camera_view.hpp"

OkAvatar::OkAvatar(OkObject *controlled, OkAvatarController *controller) {
  _controlled = controlled;
  _controller = controller;
}

OkAvatar::~OkAvatar() {
  delete _controller;
  for (std::size_t i = 0; i < _views.size(); i++) {
    delete _views[i];
  }
  _views.clear();
}

void OkAvatar::setController(OkAvatarController *controller) {
  if (_controller != controller) {
    delete _controller;
    _controller = controller;
  }
}

void OkAvatar::addView(OkCameraView *view) {
  if (view) _views.push_back(view);
}

OkCameraView *OkAvatar::viewForCamera(const OkCamera *camera) const {
  for (std::size_t i = 0; i < _views.size(); i++) {
    if (_views[i]->camera() == camera) return _views[i];
  }
  return nullptr;
}

void OkAvatar::update(float dt, const OkInputState &input,
                      OkCamera *activeCamera) {
  if (_controller && _controlled) {
    _controller->update(dt, input, *_controlled, activeCamera);
  }
  // Reposition every view relative to the (now moved) avatar, so switching
  // cameras is instant.
  for (std::size_t i = 0; i < _views.size(); i++) {
    if (_controlled) _views[i]->update(*_controlled, dt);
  }
}
