---
title: MCP server
section: Reference
nav_order: 12
---

# MCP server

Okinawa ships an optional in-engine MCP (Model Context Protocol) server. It lets an external agent connect to a running app over local HTTP and observe or drive it through the tools listed below. The public surface (`OkMcpServer`) deliberately exposes no HTTP or JSON types: all of that lives behind a pimpl, so consumers do not inherit those dependencies.

## Tools

The server exposes these tools to a connected agent:

| Tool | What it does |
| --- | --- |
| `view_frame` | Returns the current rendered frame as a PNG image, so the agent can see what is on screen. |
| `screenshot` | Writes the current frame to a PNG file on disk (for a human) and returns the path. Optional `path` (default `okinawa-screenshot.png`). |
| `press_key` | Holds a key for a duration to drive the app: W/A/S/D move, SPACE/T/R/F are actions, 1-9 switch camera, arrows turn. Args: `key`, `duration_ms` (default 120). Returns the resulting camera pose. |
| `press_keys` | Holds several keys at once (e.g. W and D for diagonal movement). Args: `keys`, `duration_ms`. Returns the resulting camera pose. |
| `look` | Rotates the active view by `yaw_deg` / `pitch_deg` (the look / mouse-move equivalent): orbits the avatar when an avatar view is active, otherwise rotates the free-fly camera. Works with physical input disabled. Returns the resulting camera pose. |
| `set_camera_pose` | Teleports and orients the active camera directly (`x`, `y`, `z`, `pitch_deg`, `yaw_deg`, `roll_deg`); omitted fields keep their current value. Returns the resulting camera pose. |
| `get_state` | Returns numeric runtime state: active camera pose, fps, scene object count, window size and resident memory. |

## Enabling it

From application code, enable the server after `OkCore::initialize()`:

```cpp
OkCore::enableMcpServer();        // binds 127.0.0.1:8765
OkCore::setIgnoreUserInput(true); // optional: MCP-only control
```

`enableMcpServer(int port = 8765)` always exists. If the engine was built without MCP support it logs a warning and does nothing, so apps compile identically with or without the server. The server binds `127.0.0.1` on the given port and logs a line such as `MCP server listening on http://127.0.0.1:8765/mcp`.

## Connecting a client

Once a build with the server enabled is running (it logs `MCP server listening on http://127.0.0.1:8765/mcp`), point your MCP client at that URL. The transport is streamable HTTP.

With the Claude Code CLI, add it on the command line:

```bash
claude mcp add --transport http okinawa http://127.0.0.1:8765/mcp
```

This registers the server (named `okinawa` here) at local scope. Reload or reconnect the client so it picks up the tools, then the tools listed above are available. Remove it later with `claude mcp remove okinawa`. The server only exists while the app runs, so start the app and reconnect each session.

## Compile-time toggle

The server is compiled in by default and guarded by `OKINAWA_WITH_MCP` (xmake option `mcp`). Exclude it from lean builds with `xmake f --mcp=n`, or be explicit with `--mcp=y`. When excluded, no MCP/HTTP code or its header-only dependencies land in the binary.

## OkMcpServer methods

`OkMcpServer` is normally managed by `OkCore`; you rarely construct it directly. Its public surface:

| Method | Purpose |
| --- | --- |
| `OkMcpServer(int port)` | Construct bound to a port. |
| `void start()` | Start the HTTP server thread. |
| `void stop()` | Stop the server and join its thread. |
| `void drainCommands()` | Run queued tool commands on the engine loop thread. |

`drainCommands()` must be called once per frame from the engine loop thread (the one holding the OpenGL context), after the scene is rendered and before the buffers are swapped. `OkCore::loop` does this for you when the server is enabled.
