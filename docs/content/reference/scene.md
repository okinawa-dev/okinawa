---
title: Scene
section: Reference
nav_order: 2
---

# Scene

`OkScene` is a container for a hierarchy of `OkObject`s. It stores only the root objects (those without a parent) and drives their per-frame step and draw, recursing into children. You add a scene to the engine through the scene handler (see [Handlers](/reference/handlers.html)).

## OkScene methods

| Method | Purpose |
| --- | --- |
| `OkScene(const std::string &name)` | Construct a named scene. |
| `void addObject(OkObject *object)` | Add a root object to the scene. |
| `void step(float dt)` | Step every object (called by the engine). |
| `void draw()` | Draw every object (called by the engine). |
| `void activate()` / `void deactivate()` | Toggle the scene's active state. |
| `bool isActive() const` | Whether the scene is active. |
| `bool isCurrent() const` | Whether the scene is the current one. |
| `size_t getObjectCount() const` | Number of root objects. |
| `const std::string &getName() const` | The scene name. |

The scene owns its objects: deleting the scene deletes the objects it holds, so do not delete added objects yourself.

## Example

```cpp
OkScene *scene = new OkScene("MainScene");

OkSceneHandler *handler = OkCore::getSceneHandler();
handler->addScene(scene, "MainScene");
handler->setScene(0);

scene->addObject(myItem);
```
