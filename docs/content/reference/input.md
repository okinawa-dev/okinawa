---
title: Input
section: Reference
nav_order: 7
---

# Input

`OkInput` reads the keyboard and mouse each frame and exposes both an edge/level query API and a digested `OkInputState`. You usually reach it through `OkCore::getInput()` inside the step callback. Keys are referenced through the platform-independent `OkKey` enum (see `input/keys.hpp`).

## OkInput methods

| Method | Purpose |
| --- | --- |
| `bool isKeyJustPressed(OkKey key) const` | True only on the frame the key goes down. |
| `bool isKeyHeld(OkKey key) const` | True while the key is held. |
| `bool isKeyJustReleased(OkKey key) const` | True only on the frame the key is released. |
| `OkInputState getState() const` | The digested per-frame input state. |
| `void injectKey(OkKey key, double durationSeconds)` | Synthesize a key press (used to drive the app programmatically). |
| `void setPhysicalInputEnabled(bool enabled)` | Enable/disable physical keyboard/mouse input. |

## OkInputState fields

`getState()` returns a struct with ready-to-use flags. Movement: `forward`, `backward`, `strafeLeft`, `strafeRight`. Rotation: `turnLeft`, `turnRight`, `turnUp`, `turnDown`. Edge-triggered actions (true only on the frame first pressed): `action1`, `action2`, `action3`, `action4`. Camera selection: `changeCamera` (-1 if none). And `exit`, set when the user asks to quit.

## Example

```cpp
void stepCallback(float deltaTime) {
  OkInput     *input  = OkCore::getInput();
  OkInputState state  = input->getState();

  if (state.exit) {
    OkCore::askForExit();
    return;
  }

  OkCamera *camera = OkCore::getCamera();
  OkPoint   forward = camera->getRotation().getForwardVector();
  if (state.forward) {
    camera->move(forward.x() * deltaTime,
                 forward.y() * deltaTime,
                 forward.z() * deltaTime);
  }
}
```
