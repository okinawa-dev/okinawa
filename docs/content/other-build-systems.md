---
title: Other build systems
section: Start
nav_order: 2
---

# Using okinawa from another build system

Okinawa is built and consumed with [xmake](/getting-started.html), which fetches
its dependencies for you. There is currently no build-system-agnostic packaging
(no CMake package config, no pkg-config, no prebuilt artifacts), so a project on
CMake, Make, Meson, and so on has to wire the engine in by hand.

Adding okinawa as a git submodule is never a problem in itself. The friction is
only at build and link time, and it is manageable: the engine is plain C++17 and
ships as a static library.

## What the engine needs

`libokinawa.a` is a static library. To link it you also need its **public**
dependencies, the ones xmake would otherwise resolve for you:

| Dependency | Notes |
| --- | --- |
| GLM | Header-only (math). |
| STB | Header-only (image loading). |
| GLFW | Window and OpenGL context; link it. |
| OpenGL | The system OpenGL library. |
| GLEW | Linux and Windows only: the OpenGL function loader. Call `glewInit()` once a context is current, or let `OkCore` do it. macOS uses the system OpenGL framework instead, so it needs no GLEW. |

The MCP server's dependencies (cpp-httplib and nlohmann/json, both header-only)
are **private**: you only need them if you compile the MCP server in (see below).

Headers are included with the `okinawa/` prefix (for example
`#include "okinawa/core/core.hpp"`), so add the engine's `src/` directory to your
include path.

On macOS, also link the `Cocoa`, `IOKit`, `CoreVideo` and `OpenGL` frameworks
(required by GLFW).

## Option 1: build okinawa with xmake, link the result (recommended)

Keep xmake only to build the engine, and consume the artifact from your own
build. This is the lowest-friction path, because xmake still fetches and builds
the dependencies for you.

```bash
git submodule add https://github.com/okinawa-dev/okinawa.cpp okinawa.cpp
xmake build -P okinawa.cpp -y okinawa
```

That produces `okinawa.cpp/build/<plat>/<arch>/<mode>/libokinawa.a`, for example
`okinawa.cpp/build/linux/x86_64/release/libokinawa.a`. In your own build:

- add `okinawa.cpp/src` to the include path,
- link that `libokinawa.a`,
- link GLFW, OpenGL, and GLEW (off macOS); on macOS add the frameworks above,
- on Linux and Windows, call `glewInit()` once a GL context is current, or go
  through `OkCore`, which does it for you.

## Option 2: compile the sources directly

If you do not want xmake at all, add the engine sources to your own build:

- compile `okinawa.cpp/src/**.cpp`,
- add both `okinawa.cpp/src` and `okinawa.cpp/src/okinawa` to the include path
  (the second resolves the engine's internal, unprefixed includes),
- provide GLM, STB, GLFW, GLEW, and OpenGL yourself,
- add the macOS frameworks on macOS.

The platform branches (Apple framework versus GLEW) key on `__APPLE__`, so they
resolve per platform with no extra flags.

### The MCP server

The optional in-engine MCP server is guarded by `OKINAWA_WITH_MCP`. If you do not
define it, `mcp-server.cpp` compiles to an empty translation unit and you need
neither cpp-httplib nor nlohmann/json. Define `OKINAWA_WITH_MCP` to compile it
in, and then add those two header-only libraries to your include path.

## Generating build files from xmake

As a shortcut, xmake can emit build files for other systems from the engine's
project:

```bash
xmake project -P okinawa.cpp -k cmake      # CMakeLists.txt
xmake project -P okinawa.cpp -k makefile   # Makefile
```

These still expect the dependencies to be resolvable, but they can be a useful
starting point rather than writing the build rules from scratch.
