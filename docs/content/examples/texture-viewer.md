---
title: Texture viewer
section: Examples
nav_order: 2
---

# Building a texture-preview overlay

This tutorial builds a small on-screen overlay that displays a texture and cycles through the available textures with a key press. The idea is general: parent a flat, textured quad to the camera so it stays fixed on screen, then swap its texture on demand. The technique is used by the wadviewer app to preview WAD textures, but nothing here is WAD-specific.

## A quad parented to the camera

An overlay element is just an `OkItem` quad attached to the camera. Attaching to the camera means its transform is relative to the view, so it stays in the same place on screen as the camera moves. Position it in front of the camera (negative Z) and off to one corner.

```cpp
OkItem *makePreviewQuad(OkCamera *camera, float width, float height) {
  float hw = width / 2.0f;
  float hh = height / 2.0f;

  std::vector<float> vertices = {
    // position           // uv
    -hw,  hh, 0.0f,        0.0f, 1.0f,  // top left
     hw,  hh, 0.0f,        1.0f, 1.0f,  // top right
     hw, -hh, 0.0f,        1.0f, 0.0f,  // bottom right
    -hw, -hh, 0.0f,        0.0f, 0.0f,  // bottom left
  };
  std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };

  OkItem *quad = new OkItem("texture_preview", vertices.data(),
                            static_cast<int>(vertices.size()),
                            indices.data(),
                            static_cast<int>(indices.size()));
  quad->attachTo(camera);
  quad->setPosition(10.0f, -7.0f, -30.0f);  // fixed in the camera's view
  return quad;
}
```

## Apply a texture

Textures are owned and shared by `OkTextureHandler` (see [Handlers](/reference/handlers.html)). Ask it for the list of loaded textures and apply one with `setTexture`:

```cpp
OkTextureHandler *handler = OkTextureHandler::getInstance();
std::vector<std::string> names = handler->getTextureNames();

OkTexture *tex = handler->getTexture(names[0]);
if (tex) {
  quad->setTexture("texture_preview", tex);
}
```

Because the GUI is created before the OpenGL context is fully ready, the wadviewer GUI defers the quad's actual creation to the first `step` call. If you build the quad after `OkCore::initialize()` and the scene is set up, you can create it directly.

## Cycle textures with a key

Drive the overlay from the step callback. The edge-triggered `action` flags on `OkInputState` are true only on the frame a key is first pressed, which is exactly what you want for cycling (one step per press, not one per frame). See [Input](/reference/input.html).

```cpp
void stepCallback(float deltaTime) {
  OkInputState state = OkCore::getInput()->getState();

  static int index = 0;
  if (state.action1) {  // next texture
    OkTextureHandler *handler = OkTextureHandler::getInstance();
    std::vector<std::string> names = handler->getTextureNames();
    if (!names.empty()) {
      index = (index + 1) % static_cast<int>(names.size());
      OkTexture *tex = handler->getTexture(names[index]);
      if (tex) {
        quad->setTexture("texture_preview", tex);
      }
    }
  }

  if (state.action2) {  // toggle the overlay
    quad->setVisible(!quad->getVisible());
  }
}
```

When the texture's dimensions differ from the quad's, you can rebuild the quad's vertex data with `quad->updateVertexData(...)` to match the texture aspect ratio, rather than recreating the item.
