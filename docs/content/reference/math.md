---
title: Math
section: Reference
nav_order: 4
---

# Math

The math types wrap GLM behind a small, explicit API. `OkPoint` is a 3D point or vector, `OkRotation` holds Euler angles (and the matrix they produce), and `OkMath` is a static helper for direction/angle conversions and look-at.

## Coordinate system

Okinawa uses a right-handed coordinate system: X points right, Y points up, Z points towards the viewer (out of the screen). The default camera sits at the origin looking down negative Z, with up along positive Y. Rotations are Euler angles in radians: pitch (X), yaw (Y), roll (Z), with pitch clamped to avoid gimbal lock. See `src/okinawa/math/readme.md` in the engine for the full conventions.

## OkPoint methods

| Method | Purpose |
| --- | --- |
| `OkPoint(float x, float y, float z)` | Construct from components. |
| `float x() / y() / z() const` | Component getters. |
| `float magnitude() const` | Vector length. |
| `OkPoint normalize() const` | Unit vector. |
| `float distance(const OkPoint &other) const` | Distance to another point. |
| `float dot(const OkPoint &other) const` | Dot product. |
| `OkPoint cross(const OkPoint &other) const` | Cross product. |
| `static OkPoint Forward() / Right() / Up()` | Basis vectors. |

`OkPoint` also supports `+`, `-`, `*` (scalar), and the compound assignment operators.

## OkRotation methods

| Method | Purpose |
| --- | --- |
| `OkRotation(float pitch, float yaw, float roll)` | Construct from Euler angles (radians). |
| `void setRotation(float x, float y, float z)` | Replace the angles. |
| `void rotate(float dx, float dy, float dz)` | Apply a delta rotation. |
| `OkPoint getForwardVector() const` | Forward direction. |
| `OkPoint getRightVector() const` | Right direction. |
| `OkPoint getUpVector() const` | Up direction. |
| `OkPoint transformPoint(const OkPoint &p) const` | Rotate a point. |

## OkMath methods

| Method | Purpose |
| --- | --- |
| `static void directionVectorToAngles(const OkPoint &dir, float &outPitch, float &outYaw)` | Decompose a direction into pitch/yaw. |
| `static OkRotation lookAt(const OkPoint &eye, const OkPoint &target, const OkPoint &up = OkPoint(0,1,0))` | Build a rotation that looks from eye to target. |

## Example

```cpp
OkPoint eye(0.0f, 100.0f, 200.0f);
OkPoint target(0.0f, 0.0f, 0.0f);
OkPoint direction = (target - eye).normalize();

float pitch, yaw;
OkMath::directionVectorToAngles(direction, pitch, yaw);
camera->setRotation(pitch, yaw, 0.0f);
```
