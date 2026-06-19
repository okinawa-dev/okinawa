---
title: Config
section: Reference
nav_order: 9
---

# Config

`OkConfig` is a typed, global key/value store for engine and application settings. It is a singleton with static accessors, seeded with defaults at startup (for example window size and graphics flags). Values are looked up by string key and grouped by type: int, float and bool maps are kept separate.

## OkConfig methods

| Method | Purpose |
| --- | --- |
| `static void setInt(const std::string &key, int value)` | Set an int value. |
| `static void setFloat(const std::string &key, float value)` | Set a float value. |
| `static void setBool(const std::string &key, bool value)` | Set a bool value. |
| `static int getInt(const std::string &key)` | Read an int value. |
| `static float getFloat(const std::string &key)` | Read a float value. |
| `static bool getBool(const std::string &key)` | Read a bool value. |
| `static void reset()` | Restore every value to its default. |

Keys are plain strings, so applications can store their own settings alongside the engine's (for example a `"viewer.debug-gizmos-visible"` flag). `reset()` is mainly useful to isolate global state between tests.

## Example

```cpp
int width  = OkConfig::getInt("window.width");
int height = OkConfig::getInt("window.height");

OkConfig::setBool("graphics.drawCameras", false);
bool drawCameras = OkConfig::getBool("graphics.drawCameras");
```
