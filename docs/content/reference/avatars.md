---
title: Avatars
section: Reference
nav_order: 8
---

# Avatars

An **avatar** is the controllable representation of the player in a scene: a
controlled object plus the input scheme (controller) that drives it. The avatar
is the controlled entity; cameras are views of it. okinawa tracks an **active
avatar** and feeds it the input each frame; swapping the active avatar (say, on
foot to a car) changes the controls in one call.

## OkAvatar

```cpp
#include "okinawa/avatar/avatar.hpp"
#include "okinawa/avatar/walk_controller.hpp"

// `prism` is an OkItem (or any OkObject) already in the scene.
OkAvatar *player = new OkAvatar(prism, new OkWalkController(5.0f));
OkCore::setActiveAvatar(player);
```

`OkAvatar` owns its controller (deletes it) but **not** the controlled object,
which belongs to the scene. `getControlledObject()` / `getController()` /
`setController()` are available.

## OkAvatarController

The input scheme. Implement it for a custom control style:

```cpp
class OkAvatarController {
  virtual void update(float dt, const OkInputState &input,
                      OkObject &avatar, OkCamera *activeCamera) = 0;
};
```

`update` runs once per frame for the active avatar. `activeCamera` lets a
controller make movement camera-relative. The engine keeps the **polled**
per-frame input model (see [Input](/reference/input.html)); discrete actions use
the edge-triggered action buttons.

## OkWalkController

A stock controller: **camera-relative ground movement** (GTA-style). W/S move
along the camera's forward (projected onto the ground), A/D strafe along its
right, and the avatar turns to face its movement. `setMoveSpeed(float)` sets the
speed in units per second. (Y is left untouched: there is no terrain-follow
yet.)

## Active avatar

`OkCore::setActiveAvatar(avatar)` / `getActiveAvatar()`. The active avatar is
updated each frame right after input and before the camera step. With **no**
active avatar (the default), input keeps the free-fly camera behaviour, so the
camera works as a debug fly-through.

The avatar's cameras (third-person, top-down) are views bound to it; see the
camera rig.
