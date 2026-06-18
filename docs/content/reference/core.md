---
title: Core
section: Reference
nav_order: 1
---

# Core

`OkCore` owns the window, the OpenGL context, the cameras and the main loop. It is a static class (the constructor is deleted), so every entry point is called through `OkCore::`. It also exposes the engine's scene handler, input and (optionally) the in-engine MCP server.

`OkCamera` is a transformable view onto the scene. It derives from `OkObject` (see [Items](/reference/items.html) for the shared transform API) and adds projection and view matrices.

## OkCore methods

| Method | Purpose |
| --- | --- |
| `static bool initialize()` | Create the window and OpenGL context. Returns false on failure. |
| `static void loop(step, draw)` | Run the main loop, calling the step and draw callbacks each frame. |
| `static void askForExit()` | Request the loop to end (typically from the step callback). |
| `static void exit()` | Tear down and exit. |
| `static OkSceneHandler *getSceneHandler()` | Access the scene handler. |
| `static OkCamera *getCamera()` | The current camera. |
| `static OkInput *getInput()` | The input subsystem. |
| `static void addCamera(OkCamera *camera)` | Register an additional camera. |
| `static void switchCamera(int index)` | Make the camera at `index` current. |
| `static void enableMcpServer(int port = 8765)` | Start the in-engine MCP server (see [MCP server](/reference/mcp.html)). |
| `static void setIgnoreUserInput(bool ignore)` | Ignore physical input (MCP-driven instances). |

The loop callbacks share the signature `void(float deltaTime)`.

## OkCamera methods

| Method | Purpose |
| --- | --- |
| `OkCamera(name, width, height)` | Construct a camera for a given framebuffer size. |
| `void setPerspective(fovDegrees, near, far)` | Set the projection. |
| `const glm::mat4 &getView() const` | The view matrix. |
| `const glm::mat4 &getProjection() const` | The projection matrix. |

`OkCamera` also inherits `setPosition`, `setRotation` and the rest of the `OkObject` transform API.

## Example

```cpp
if (!OkCore::initialize()) {
  return 1;
}

OkCamera *camera = OkCore::getCamera();
camera->setPosition(0.0f, 0.0f, 5.0f);
camera->setPerspective(45.0f, 0.1f, 1000.0f);

OkCore::loop(stepCallback, drawCallback);
```
