#ifndef OK_GRAPH_HPP
#define OK_GRAPH_HPP

#include "../core/object.hpp"
#include "../math/point.hpp"

#include <vector>

class OkItem;

/**
 * @brief A generic graph of nodes and edges that can be rendered. Build it with
 *        addNode / addEdge, then rebuild() to (re)generate its geometry. It is a
 *        scene object (transform + hierarchy); rendering is delegated to internal
 *        OkItems attached as children: edges as GL_LINES, nodes as GL_POINTS,
 *        each with its own colour. The node/edge data stays queryable (a base for
 *        pathfinding and, later, spline/tracker paths).
 */
class OkGraph : public OkObject {
public:
  explicit OkGraph(const std::string &name);
  ~OkGraph() override;

  // Build / mutate. addNode returns the new node's index.
  int  addNode(const OkPoint &position);
  void addEdge(int a, int b);
  void clear();

  // (Re)generate the renderable geometry from the current nodes/edges.
  void rebuild();

  // Render options.
  void setEdgeColor(float r, float g, float b);
  void setNodeColor(float r, float g, float b);
  void setShowEdges(bool show);
  void setShowNodes(bool show);
  void setNodeSize(float pixels) { _nodeSize = pixels; }

  // Queries.
  int            getNodeCount() const { return static_cast<int>(_nodes.size()); }
  int            getEdgeCount() const { return static_cast<int>(_edges.size()); }
  const OkPoint &getNode(int i) const { return _nodes[i]; }

  // OkObject hooks. drawSelf only sets GL point size; the child items draw.
  void drawSelf() override;
  void stepSelf(float dt) override { (void)dt; }
  void updateTransformSelf() override {}

private:
  struct Edge {
    int a;
    int b;
  };

  std::vector<OkPoint> _nodes;
  std::vector<Edge>    _edges;

  OkItem *_edgesItem;  // GL_LINES, owned
  OkItem *_nodesItem;  // GL_POINTS, owned

  float _edgeColor[3];
  float _nodeColor[3];
  bool  _showEdges;
  bool  _showNodes;
  float _nodeSize;

  void destroyItems();
};

#endif  // OK_GRAPH_HPP
