---
title: Items
section: Reference
nav_order: 3
---

# Items

`OkItem` is a renderable mesh: vertex data, indices, an optional texture and the shared transform (position, rotation, scale, hierarchy). It derives from `OkObject`, which provides the transform API used across the engine. `OkItemGroup` bundles several items so they move and render as one unit and can be tagged for selective visibility. `OkTexture` wraps a loaded GPU texture.

## OkObject (shared transform)

Every drawable inherits these:

| Method | Purpose |
| --- | --- |
| `void setPosition(float x, float y, float z)` | Set the local position. |
| `void move(float dx, float dy, float dz)` | Translate by a delta. |
| `void setRotation(float x, float y, float z)` | Set Euler rotation (radians). |
| `void rotate(float dx, float dy, float dz)` | Rotate by a delta (radians). |
| `void setScaling(float x, float y, float z)` | Set the scale. |
| `void attachTo(OkObject *parent)` | Parent this object (transforms compose). |
| `void attach(OkObject *child)` | Add a child to this object. |
| `void detachFromParent()` | Detach from the parent. |
| `void setDrawOriginAxis(bool)` | Toggle the debug origin axis gizmo. |

## OkItem methods

| Method | Purpose |
| --- | --- |
| `OkItem(name, vertexData, vertexCount, indexData, indexCount)` | Construct from interleaved vertex data and indices. |
| `void setWireframe(bool)` | Draw as wireframe. |
| `void setVisible(bool)` | Show or hide the item. |
| `void setDrawMode(GLenum mode)` | Set the GL primitive (`GL_TRIANGLES`, `GL_LINES`, ...). |
| `void loadTextureFromFile(const std::string &path)` | Load and apply a texture from disk. |
| `void setTexture(const std::string &name, OkTexture *tex)` | Apply an already-loaded texture. |
| `void updateVertexData(float *data, long count)` | Replace the vertex data in place. |
| `float getRadius() const` | The mesh's maximum dimension. |

## OkItemGroup methods

| Method | Purpose |
| --- | --- |
| `OkItemGroup(name)` | Construct an empty group. |
| `void addItem(OkItem *item, const std::string &tag)` | Add an item with a tag. |
| `std::vector<OkItem *> getItemsWithTag(const std::string &tag)` | Items carrying a tag. |
| `int getItemCountWithTag(const std::string &tag)` | Count items with a tag. |
| `void setVisible(bool)` | Show or hide every item. |
| `void setWireframe(bool)` | Wireframe every item. |

## Example

```cpp
float vertices[] = {
  // position           // uv
   0.5f,  0.5f, 0.0f,    1.0f, 1.0f,
   0.5f, -0.5f, 0.0f,    1.0f, 0.0f,
  -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,
  -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,
};
unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

OkItem *quad = new OkItem("quad", vertices, 20, indices, 6);
quad->setPosition(0.0f, 0.0f, -5.0f);
quad->setWireframe(true);
scene->addObject(quad);
```
