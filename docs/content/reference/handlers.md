---
title: Handlers
section: Reference
nav_order: 9
---

# Handlers

Handlers are the engine's coordinators. `OkSceneHandler` owns the collection of scenes and decides which one is current. `OkTextureHandler` is a singleton that loads, stores and reference-counts textures so the same texture is shared rather than reloaded.

## OkSceneHandler

Reach the scene handler through `OkCore::getSceneHandler()`; the engine creates it for you.

| Method | Purpose |
| --- | --- |
| `void addScene(OkScene *scene, const std::string &name)` | Register a scene. |
| `void insertScene(OkScene *scene, const std::string &name, int index)` | Insert at a position. |
| `void setScene(int index)` | Make the scene at `index` current. |
| `void advance()` / `void goBack()` | Step to the next/previous scene. |
| `OkScene *getCurrentScene() const` | The current scene. |
| `int getCurrentSceneIndex() const` | Its index. |
| `int getSceneCount() const` | Number of registered scenes. |

## OkTextureHandler

A singleton: call `OkTextureHandler::getInstance()`.

| Method | Purpose |
| --- | --- |
| `static OkTextureHandler *getInstance()` | The singleton instance. |
| `OkTexture *createTextureFromFile(const std::string &path)` | Load and store a texture. |
| `OkTexture *createTextureFromRawData(name, data, width, height, channels)` | Store a texture from raw pixels. |
| `OkTexture *getTexture(const std::string &name)` | Look up a stored texture (nullptr if absent). |
| `std::vector<std::string> getTextureNames() const` | Names of all stored textures. |
| `void addReference(name)` / `void removeReference(name)` | Adjust the reference count. |

## Example

```cpp
OkSceneHandler *handler = OkCore::getSceneHandler();
handler->addScene(new OkScene("MainScene"), "MainScene");
handler->setScene(0);

OkTextureHandler *textures = OkTextureHandler::getInstance();
OkTexture *wall = textures->createTextureFromFile("assets/wall.png");
```
