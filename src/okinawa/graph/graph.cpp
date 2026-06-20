#include "graph.hpp"

#include "../core/gl_config.hpp"  // GL enums + glPointSize
#include "../item/item.hpp"

#include <vector>

OkGraph::OkGraph(const std::string &name) : OkObject(name) {
  _edgesItem    = nullptr;
  _nodesItem    = nullptr;
  _edgeColor[0] = 1.0f;
  _edgeColor[1] = 1.0f;
  _edgeColor[2] = 1.0f;
  _nodeColor[0] = 1.0f;
  _nodeColor[1] = 1.0f;
  _nodeColor[2] = 1.0f;
  _showEdges    = true;
  _showNodes    = false;
  _nodeSize     = 4.0f;
}

OkGraph::~OkGraph() {
  destroyItems();
}

void OkGraph::destroyItems() {
  if (_edgesItem) {
    _edgesItem->detachFromParent();
    delete _edgesItem;
    _edgesItem = nullptr;
  }
  if (_nodesItem) {
    _nodesItem->detachFromParent();
    delete _nodesItem;
    _nodesItem = nullptr;
  }
}

int OkGraph::addNode(const OkPoint &position) {
  _nodes.push_back(position);
  return static_cast<int>(_nodes.size()) - 1;
}

void OkGraph::addEdge(int a, int b) {
  Edge e;
  e.a = a;
  e.b = b;
  _edges.push_back(e);
}

void OkGraph::clear() {
  _nodes.clear();
  _edges.clear();
}

void OkGraph::setEdgeColor(float r, float g, float b) {
  _edgeColor[0] = r;
  _edgeColor[1] = g;
  _edgeColor[2] = b;
  if (_edgesItem) {
    _edgesItem->setWireframeColor(r, g, b);
  }
}

void OkGraph::setNodeColor(float r, float g, float b) {
  _nodeColor[0] = r;
  _nodeColor[1] = g;
  _nodeColor[2] = b;
  if (_nodesItem) {
    _nodesItem->setWireframeColor(r, g, b);
  }
}

void OkGraph::setShowEdges(bool show) {
  _showEdges = show;
  if (_edgesItem) {
    _edgesItem->setVisible(show);
  }
}

void OkGraph::setShowNodes(bool show) {
  _showNodes = show;
  if (_nodesItem) {
    _nodesItem->setVisible(show);
  }
}

void OkGraph::rebuild() {
  destroyItems();
  if (_nodes.empty()) {
    return;
  }

  // Shared vertex buffer: position + a dummy UV (OkItem's vertex stride is 5).
  std::vector<float> verts;
  verts.reserve(_nodes.size() * 5);
  for (std::size_t i = 0; i < _nodes.size(); i++) {
    verts.push_back(_nodes[i].x());
    verts.push_back(_nodes[i].y());
    verts.push_back(_nodes[i].z());
    verts.push_back(0.0f);
    verts.push_back(0.0f);
  }

  // Edges as a line list.
  if (!_edges.empty()) {
    std::vector<unsigned int> edgeIdx;
    edgeIdx.reserve(_edges.size() * 2);
    for (std::size_t i = 0; i < _edges.size(); i++) {
      edgeIdx.push_back(static_cast<unsigned int>(_edges[i].a));
      edgeIdx.push_back(static_cast<unsigned int>(_edges[i].b));
    }
    _edgesItem = new OkItem(getName() + "_edges", verts.data(),
                            static_cast<long>(verts.size()), edgeIdx.data(),
                            static_cast<long>(edgeIdx.size()));
    _edgesItem->setDrawMode(GL_LINES);
    _edgesItem->setWireframeColor(_edgeColor[0], _edgeColor[1], _edgeColor[2]);
    _edgesItem->setVisible(_showEdges);
    attach(_edgesItem);
  }

  // Nodes as a point list.
  std::vector<unsigned int> nodeIdx;
  nodeIdx.reserve(_nodes.size());
  for (std::size_t i = 0; i < _nodes.size(); i++) {
    nodeIdx.push_back(static_cast<unsigned int>(i));
  }
  _nodesItem = new OkItem(getName() + "_nodes", verts.data(),
                          static_cast<long>(verts.size()), nodeIdx.data(),
                          static_cast<long>(nodeIdx.size()));
  _nodesItem->setDrawMode(GL_POINTS);
  _nodesItem->setWireframeColor(_nodeColor[0], _nodeColor[1], _nodeColor[2]);
  _nodesItem->setVisible(_showNodes);
  attach(_nodesItem);
}

void OkGraph::drawSelf() {
  // Children (the nodes item) draw right after this; set their point size here.
  glPointSize(_nodeSize);
}
