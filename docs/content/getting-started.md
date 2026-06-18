---
title: Getting started
section: Start
nav_order: 1
---

# Getting started

Okinawa is consumed from source. The recommended way to add it to a project is as a git submodule built with [xmake](https://xmake.io), which also fetches the engine's third-party dependencies (glm, glfw, stb, opengl) automatically on the first build.

Not using xmake? See [Other build systems](/other-build-systems.html) for consuming the engine from CMake, Make, and similar.

## Add the engine as a submodule

```bash
git submodule add https://github.com/okinawa-dev/okinawa.cpp okinawa.cpp
git submodule update --init
```

## Wire it into your xmake build

In your project's `xmake.lua`, include the submodule and depend on the `okinawa` target:

```lua
includes("okinawa.cpp")

target("myapp")
    set_kind("binary")
    add_deps("okinawa")
    add_files("src/*.cpp")
```

`includes("okinawa.cpp")` pulls the engine target into your build tree, so editing the engine and rebuilding your app picks the changes up directly (no binary package step).

## Open a window and render something

The entry points live in `OkCore` (a static class, never instantiated). Call `OkCore::initialize()` to create the window and OpenGL context, build a scene through `OkSceneHandler`, add objects to it, then hand two callbacks (a step callback and a draw callback) to `OkCore::loop`. The loop runs until the user asks to exit.

```cpp
#include "okinawa/core/core.hpp"
#include "okinawa/core/camera.hpp"
#include "okinawa/scene/scene.hpp"
#include "okinawa/handlers/scenes.hpp"
#include "okinawa/input/input.hpp"

void stepCallback(float deltaTime) {
  OkInputState state = OkCore::getInput()->getState();
  if (state.exit) {
    OkCore::askForExit();
  }
  // Move/update objects here using deltaTime.
}

void drawCallback(float deltaTime) {
  // Per-frame draw hook (the scene draws itself).
}

int main() {
  if (!OkCore::initialize()) {
    return 1;
  }

  // Build a scene and register it with the engine's scene handler.
  OkScene        *scene        = new OkScene("MainScene");
  OkSceneHandler *sceneHandler = OkCore::getSceneHandler();
  sceneHandler->addScene(scene, "MainScene");
  sceneHandler->setScene(0);

  // A camera is created by the engine; position it before the loop.
  OkCamera *camera = OkCore::getCamera();
  camera->setPosition(0.0f, 0.0f, 5.0f);
  camera->setPerspective(45.0f, 0.1f, 1000.0f);

  // Add your objects to the scene with scene->addObject(...).

  OkCore::loop(stepCallback, drawCallback);

  delete scene;  // child objects are deleted by the scene destructor
  return 0;
}
```

From here, see [Creating objects on the fly](/examples/objects-on-the-fly.html) to add and animate items.

## Previewing the docs locally

This site uses root-relative links (for example `/reference/core.html`), so the generated pages must be served over HTTP, not opened via `file://`. Build the site and serve it:

```bash
xmake run -P docs/tool okinawa-docs && python3 -m http.server -d docs/dist
```

Then open the printed `http://localhost:8000/` URL. Opening the files directly from disk breaks the links.
