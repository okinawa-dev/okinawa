---
title: Third-person avatar
section: Examples
nav_order: 3
---

# Third-person avatar

The smallest full third-person setup: a box as the avatar plus a camera that
follows and orbits it. See [Avatars](/reference/avatars.html) for the full model.

## Avatar + camera

Load a model as the controlled object, drive it with an `OkGroundController`, and
add an `OkThirdPersonCamera` that follows it. The controller moves the avatar
relative to that camera, so it walks "into the screen".

```cpp
#include "okinawa/avatar/avatar.hpp"
#include "okinawa/avatar/controllers/ground_controller.hpp"
#include "okinawa/cameras/third_person_camera.hpp"

// A person-sized prism as the avatar.
OkItem *prism = OkWavefrontImporter::importFile("assets/cube.obj");
prism->setScaling(0.5f, 1.8f, 0.5f);
scene->addObject(prism);

OkGroundController *controller = new OkGroundController(8.0f);  // units/sec
OkAvatar           *player     = new OkAvatar(prism, controller);

OkThirdPersonCamera *cam = new OkThirdPersonCamera("third", 800, 600);
OkCore::addCamera(cam);               // selectable with key 1
controller->setReferenceCamera(cam);  // movement is relative to this camera
player->addCamera(cam);               // the camera follows the avatar
OkCore::setActiveAvatar(player);
```

That is all the wiring. The engine updates the active avatar each frame: WASD
moves the prism relative to the camera and the mouse orbits it. No step-callback
code is needed for movement.

## Add a debug top-down

Drop in an `OkTopDownCamera` as a second view to watch from above while the
controls stay exactly the same (control is independent of the rendered camera):

```cpp
#include "okinawa/cameras/top_down_camera.hpp"

OkTopDownCamera *top = new OkTopDownCamera("top", 800, 600, 400.0f);  // height
OkCore::addCamera(top);   // selectable with key 2
player->addCamera(top);   // follows the avatar from straight above
```

Press key 1 for third person, key 2 for the overhead view; the avatar moves the
same way in both.
