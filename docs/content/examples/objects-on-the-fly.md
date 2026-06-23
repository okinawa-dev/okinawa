---
title: Objects on the fly
section: Examples
nav_order: 1
---

# Objects on the fly

This tutorial creates two `OkItem` quads at runtime, parents one to the other with `attachTo`, and rotates the parent every frame from the step callback. Because the second quad is a child, it inherits the parent's rotation: spin the parent and the child orbits with it. See [Items](/reference/items.html) for the full transform API.

## Build the meshes

An `OkItem` takes interleaved vertex data (position plus UV) and an index buffer. Here a simple quad (two triangles) is the whole mesh:

```cpp
std::vector<float> vertices = {
  // positions          // texture coords
   0.5f,  0.5f, 0.0f,    1.0f, 1.0f,  // top right
   0.5f, -0.5f, 0.0f,    1.0f, 0.0f,  // bottom right
  -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,  // bottom left
  -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,  // top left
};

std::vector<unsigned int> indices = {
  0, 1, 3,  // first triangle
  1, 2, 3   // second triangle
};
```

## Create, configure and parent

Create both items, draw them as wireframe, add the first to the scene and attach the second to the first. Positions set after `attachTo` are relative to the parent.

```cpp
OkItem *item = new OkItem("quad", vertices.data(),
                          static_cast<int>(vertices.size()),
                          indices.data(),
                          static_cast<int>(indices.size()));
item->setWireframe(true);

OkItem *item2 = new OkItem("quad2", vertices.data(),
                           static_cast<int>(vertices.size()),
                           indices.data(),
                           static_cast<int>(indices.size()));
item2->setWireframe(true);
item2->rotate(0.0f, glm::radians(90.0f), 0.0f);

scene->addObject(item);
item2->attachTo(item);                 // item2 is now a child of item

item->setPosition(-2.0f, 0.0f, -10.0f);  // left quad, in front of the camera
item2->setPosition(2.0f, 0.0f, 0.0f);    // relative to the parent
```

The scene owns `item` (added with `addObject`), and `item` owns `item2` (its child), so the scene destructor cleans both up. Do not delete them yourself.

## Rotate the parent each frame

In the step callback, rotate the parent around its Y axis. `deltaTime` keeps the motion frame-rate independent. The child orbits along because the transforms compose.

```cpp
void stepCallback(float deltaTime) {
  if (OkCore::getInput()->getState().exit) {
    OkCore::askForExit();
    return;
  }

  // Spin the parent; the attached child follows.
  item->rotate(0.0f, glm::radians(0.1f * deltaTime), 0.0f);
}
```

Run the app and you will see the parent quad turning in place with the child quad swinging around it.
