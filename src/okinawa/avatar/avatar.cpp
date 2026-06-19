#include "avatar.hpp"

OkAvatar::OkAvatar(OkObject *controlled, OkAvatarController *controller) {
  _controlled = controlled;
  _controller = controller;
}

OkAvatar::~OkAvatar() {
  delete _controller;
}

void OkAvatar::setController(OkAvatarController *controller) {
  if (_controller != controller) {
    delete _controller;
    _controller = controller;
  }
}

void OkAvatar::update(float dt, const OkInputState &input,
                      OkCamera *activeCamera) {
  if (_controller && _controlled) {
    _controller->update(dt, input, *_controlled, activeCamera);
  }
}
