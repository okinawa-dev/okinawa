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

A stock controller: **camera-relative ground movement**. W/S move
along the camera's forward (projected onto the ground), A/D strafe along its
right, and the avatar turns to face its movement. `setMoveSpeed(float)` sets the
speed in units per second. (Y is left untouched: there is no terrain-follow
yet.)

## Active avatar

`OkCore::setActiveAvatar(avatar)` / `getActiveAvatar()`. The active avatar is
updated each frame right after input and before the camera step. With **no**
active avatar (the default), input keeps the free-fly camera behaviour, so the
camera works as a debug fly-through.

## Camera rig

An avatar's cameras are **views** bound to it: each is an `OkCameraView` that
repositions its camera relative to the avatar every frame. Register each view's
camera with `OkCore::addCamera` so the **number keys** switch between them, and
add the view to the avatar with `addView`. The active view also receives the
mouse (others ignore it); with no active avatar the mouse drives the free-fly
camera as before.

```cpp
OkCamera *cam1 = new OkCamera("third-person", w, h);
OkCamera *cam2 = new OkCamera("top-down", w, h);
OkCore::addCamera(cam1);   // key 1
OkCore::addCamera(cam2);   // key 2

OkThirdPersonView *follow = new OkThirdPersonView(cam1);
OkTopDownView     *map    = new OkTopDownView(cam2);
map->setBounds(minX, minZ, maxX, maxZ, groundY);  // the sector to frame

player->addView(follow);
player->addView(map);
```

Stock views:

- **`OkThirdPersonView`** — orbits behind/above the avatar and looks at it; the
  mouse orbits it (yaw free, pitch clamped). Because it looks toward the avatar,
  a camera-relative controller walks the avatar "into the screen".
- **`OkTopDownView`** — straight down over a ground rectangle (the sector,
  e.g. the current chunk), at the height that frames it
  (`computeHeight(sizeX, sizeZ, fov, margin)`); ignores the mouse. Set the
  rectangle with `setBounds`.

