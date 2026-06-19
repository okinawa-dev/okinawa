---
title: Textures
section: Reference
nav_order: 5
---

# Textures

A texture is an `OkTexture` (an OpenGL texture wrapper). Items reference one,
and the engine caches textures by name through `OkTextureHandler` so the same
image is shared, not reloaded, across items.

## Applying a texture to an item

The simplest way: load straight from a file onto the item.

```cpp
item->loadTextureFromFile("assets/tile.png");
```

`loadTextureFromFile` drops any previous texture on the item, asks
`OkTextureHandler` to load (or reuse) the image, and assigns it. The path
doubles as the texture's name for caching and reference counting.

To assign a texture you already hold, use `setTexture(name, tex)` — it releases
the item's previous texture reference and adopts the new one:

```cpp
OkTexture *tex = OkTextureHandler::getInstance()->getTexture("assets/tile.png");
item->setTexture("assets/tile.png", tex);
```

## OkTexture

```cpp
#include "okinawa/item/texture.hpp"

OkTexture fromFile("assets/tile.png");                 // load from disk
OkTexture blank(256, 256, 4);                          // empty RGBA
OkTexture fromData(pixels, 256, 256, 4);               // from raw bytes
```

Useful queries: `isLoaded()`, `getWidth()`, `getHeight()`, `getChannels()`,
`getPath()`. `OkTexture` is non-copyable (it owns a GL handle); pass it by
pointer. `bind()` / `OkTexture::unbind()` are used by the renderer.

## Caching

Textures are cached and reference-counted by **OkTextureHandler**, a singleton:
`OkItem::loadTextureFromFile` goes through it so the same image is shared, not
reloaded, across items, and freed only when the last item drops it. Its full
API is documented under
[OkTextureHandler](/reference/handlers.html#oktexturehandler) in the Handlers
reference.

Prefer `OkItem::loadTextureFromFile` / `setTexture` for everyday use; reach for
the handler directly only when you build textures from raw data or share one
explicitly across items.
