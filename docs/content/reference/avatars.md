---
title: Avatars
section: Reference
nav_order: 8
---

# Avatars

An **avatar** is the controllable representation of the player: a controlled
object plus the input scheme (controller) that drives it, plus a **rig of
cameras** that observe it. The avatar is the controlled entity; cameras are
views of it.

**Render and control are independent.** Switching the rendered camera (the
number keys) never changes the controls: the controller carries its own
reference frame, so you can watch from a debug top-down while the avatar keeps
moving exactly as in third person.

## Minimal example: a prism in third person

The smallest full setup: a box as the avatar, a third-person camera following it.

```cpp
OkItem *prism = OkWavefrontImporter::importFile("assets/cube.obj");
prism->setScaling(0.5f, 1.8f, 0.5f);
scene->addObject(prism);

OkGroundController *controller = new OkGroundController(8.0f);
OkAvatar           *player     = new OkAvatar(prism, controller);

OkThirdPersonCamera *cam = new OkThirdPersonCamera("third", 800, 600);
OkCore::addCamera(cam);             // selectable with key 1
controller->setReferenceCamera(cam);  // move relative to the camera
player->addCamera(cam);             // camera follows the avatar
OkCore::setActiveAvatar(player);
```

WASD moves the prism relative to the camera and the mouse orbits it. That is a
complete third-person controller.

## OkAvatar

```cpp
#include "okinawa/avatar/avatar.hpp"
#include "okinawa/avatar/controllers/ground_controller.hpp"
#include "okinawa/cameras/third_person_camera.hpp"
#include "okinawa/cameras/top_down_camera.hpp"

OkGroundController *controller = new OkGroundController(8.0f);
OkAvatar           *player     = new OkAvatar(prism, controller);

OkThirdPersonCamera *third = new OkThirdPersonCamera("third", w, h);
OkTopDownCamera     *top   = new OkTopDownCamera("top", w, h, 400.0f);
OkCore::addCamera(third);   // key 1
OkCore::addCamera(top);     // key 2

controller->setReferenceCamera(third);  // control is relative to this camera
player->addCamera(third);               // rig: cameras follow the avatar
player->addCamera(top);
OkCore::setActiveAvatar(player);
```

`OkAvatar` owns its controller (deletes it) but **not** the controlled object
(the scene owns it) nor the rig cameras (`OkCore` owns them). It updates the
controller and repositions every rig camera each frame, so non-rendered cameras
still track the avatar (their gizmos show).

## Controllers

`OkAvatarController::update(dt, input, controlled)` — no camera is passed in; the
controller obtains its own reference frame. Input stays **polled** per frame
(see [Input](/reference/input.html)); discrete actions are edge-triggered.

**`OkGroundController`** — stock controller: movement on the ground plane (XZ)
relative to a reference frame, turning the object to face its movement (character,
vehicle, ...). The frame is selectable, which is what keeps control independent
of the rendered camera:

- `setReferenceCamera(cam)` — relative to that camera (the usual gameplay camera).
- `setUseActiveCamera(true)` — relative to the active rendered camera
  (room-relative control, fixed-camera games).
- neither — relative to the controlled object's own facing.

## Cameras

Camera behaviours are `OkCamera` subclasses. `OkCamera` has two virtuals:
`updateForTarget(target, dt)` (reposition for what it observes; base does
nothing) and `look(yawDeg, pitchDeg)` (base: free-fly rotate, pitch clamped).

- **`OkThirdPersonCamera`** — orbits behind/above the target and looks at it;
  the mouse/look orbits it (pitch clamped).
- **`OkTopDownCamera`** — stays straight above the target, perpendicular, north
  (+Z) up, at a fixed height; follows it; ignores the mouse. Debug/map view.
- **`OkFixedCamera`** — static "Resident Evil" camera: fixed world position,
  optionally re-aims at the target. Combine with `setUseActiveCamera` for
  room-relative control.
- **`OkSpectatorCamera`** — free-fly: ignores the target and flies from the
  input state; mouse rotates it. Use with no active avatar for a debug
  fly-through.

## Active avatar

`OkCore::setActiveAvatar(avatar)` / `getActiveAvatar()`. The active avatar is
updated each frame after input. Swapping it (on foot -> car) changes controls
and rig in one call. With **no** active avatar, the current camera still runs
its own behaviour (e.g. a spectator flies). `OkCore::clearCameras()` lets a game
drop the seeded default camera and install its own set.
