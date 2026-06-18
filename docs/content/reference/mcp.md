---
title: MCP server
section: Reference
nav_order: 8
---

# MCP server

Okinawa ships an optional in-engine MCP (Model Context Protocol) server. It lets an external agent connect to a running app over local HTTP and observe or drive it through MCP tools (v1: `view_frame`, which returns the rendered frame as an image). The public surface (`OkMcpServer`) deliberately exposes no HTTP or JSON types: all of that lives behind a pimpl, so consumers do not inherit those dependencies.

## Enabling it

From application code, enable the server after `OkCore::initialize()`:

```cpp
OkCore::enableMcpServer();        // binds 127.0.0.1:8765
OkCore::setIgnoreUserInput(true); // optional: MCP-only control
```

`enableMcpServer(int port = 8765)` always exists. If the engine was built without MCP support it logs a warning and does nothing, so apps compile identically with or without the server. The server binds `127.0.0.1` on the given port and logs a line such as `MCP server listening on http://127.0.0.1:8765/mcp`.

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
