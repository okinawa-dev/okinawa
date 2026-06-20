---
title: Graph
section: Reference
nav_order: 9
---

# Graph

`OkGraph` is a generic graph of **nodes and edges** that renders itself. Build it
with `addNode` / `addEdge`, call `rebuild()`, and add it to the scene. Use it for
navigation graphs, waypoints, pathfinding debug, and (later) spline/tracker
paths. The node/edge data stays queryable, so the same graph can drive logic, not
just visuals.

```cpp
#include "okinawa/graph/graph.hpp"

OkGraph *g = new OkGraph("nav");
int a = g->addNode(OkPoint(0, 0, 0));
int b = g->addNode(OkPoint(10, 0, 0));
g->addEdge(a, b);

g->setEdgeColor(1.0f, 0.55f, 0.0f);  // orange edges
g->setShowNodes(true);               // also draw the nodes
g->setNodeColor(1.0f, 1.0f, 1.0f);
g->setNodeSize(5.0f);
g->rebuild();                        // (re)generate geometry after changes
scene->addObject(g);
```

## Render modes

`setRenderMode(...)` picks how it is drawn (then `rebuild()`):

- **`RENDER_LINES`** (default) — edges as `GL_LINES`, nodes as `GL_POINTS`
  (`setNodeSize`, pixels). Cheap, thin.
- **`RENDER_POLYGONS`** — edges as filled ribbons (`setEdgeWidth`, metres),
  nodes as filled quads (`setNodeMarkerSize`, metres). Thicker, more visible,
  more geometry.

Both honour `setEdgeColor` / `setNodeColor` and `setShowEdges` / `setShowNodes`.

## How it renders

`OkGraph` is an [object](/reference/items.html) (transform + hierarchy). It does
not draw geometry itself; `rebuild()` composes internal `OkItem`s attached as
children (one for edges, one for nodes), reusing the standard item rendering and
per-item colour. Mutate the graph and call `rebuild()` to refresh.

## API

| Member | Purpose |
|---|---|
| `addNode(OkPoint)` → `int` | Add a node, returns its index. |
| `addEdge(int a, int b)` | Connect two nodes by index. |
| `clear()` | Remove all nodes and edges. |
| `rebuild()` | Regenerate the renderable geometry. |
| `setRenderMode(RenderMode)` | `RENDER_LINES` or `RENDER_POLYGONS`. |
| `setEdgeColor` / `setNodeColor` | RGB colours. |
| `setShowEdges` / `setShowNodes` | Toggle each layer. |
| `setNodeSize(float)` | Point size in pixels (RENDER_LINES). |
| `setEdgeWidth(float)` / `setNodeMarkerSize(float)` | Ribbon / quad size in metres (RENDER_POLYGONS). |
| `getNodeCount` / `getEdgeCount` / `getNode(i)` | Queries. |
