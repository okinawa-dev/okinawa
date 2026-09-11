#include "meshes.hpp"

#include "../item/item.hpp"
#include "../utils/logger.hpp"

#include <cmath>
#include <cstring>
#include <vector>

OkMeshHandler *OkMeshHandler::_instance = nullptr;

OkSharedMesh::~OkSharedMesh() {
  if (vbo != 0) {
    glDeleteBuffers(1, &vbo);
  }
  if (ebo != 0) {
    glDeleteBuffers(1, &ebo);
  }
  delete[] vertices;
  delete[] indices;
}

OkMeshHandler *OkMeshHandler::getInstance() {
  if (_instance == nullptr) {
    _instance = new OkMeshHandler();
  }
  return _instance;
}

OkMeshHandler::~OkMeshHandler() {
  std::map<std::string, Entry>::iterator it;
  for (it = _meshes.begin(); it != _meshes.end(); ++it) {
    delete it->second.mesh;
  }
  _meshes.clear();
}

OkSharedMesh *OkMeshHandler::acquire(const std::string &key,
                                     const float *vertexData, long vertexCount,
                                     const unsigned int *indexData,
                                     long indexCount, int vertexStride) {
  std::map<std::string, Entry>::iterator found = _meshes.find(key);
  if (found != _meshes.end()) {
    found->second.refCount++;
    return found->second.mesh;
  }
  if (vertexData == nullptr || indexData == nullptr || indexCount <= 0) {
    return nullptr;
  }

  // The same expansion an item does for itself: the caller's layout in,
  // the item layout out, with the normals worked out from the triangle
  // list where the caller supplies none. It lives on OkItem because that
  // is what reads the result, and it is static because it is arithmetic
  // over two arrays and belongs to no particular item.
  std::vector<float> expanded;
  OkItem::expandVertices(vertexData, vertexCount, indexData, indexCount,
                         vertexStride, &expanded);
  if (expanded.empty()) {
    return nullptr;
  }

  auto *mesh        = new OkSharedMesh();
  mesh->numVertices = static_cast<long>(expanded.size());
  mesh->vertices    = new float[expanded.size()];
  std::memcpy(mesh->vertices, expanded.data(), expanded.size() * sizeof(float));
  mesh->numIndices = indexCount;
  mesh->indices    = new unsigned int[indexCount];
  std::memcpy(mesh->indices, indexData,
              static_cast<size_t>(indexCount) * sizeof(unsigned int));

  // The bounding sphere belongs to the mesh, not to whoever drew it
  // first: every item sharing this geometry reaches exactly as far.
  const long HOW_MANY = mesh->numVertices / OkItem::VERTEX_STRIDE;
  float      minX     = mesh->vertices[0];
  float      maxX     = minX;
  float      minY     = mesh->vertices[1];
  float      maxY     = minY;
  float      minZ     = mesh->vertices[2];
  float      maxZ     = minZ;
  for (long i = 1; i < HOW_MANY; i++) {
    const long at = i * OkItem::VERTEX_STRIDE;
    minX          = std::min(minX, mesh->vertices[at]);
    maxX          = std::max(maxX, mesh->vertices[at]);
    minY          = std::min(minY, mesh->vertices[at + 1]);
    maxY          = std::max(maxY, mesh->vertices[at + 1]);
    minZ          = std::min(minZ, mesh->vertices[at + 2]);
    maxZ          = std::max(maxZ, mesh->vertices[at + 2]);
  }
  const float width  = maxX - minX;
  const float height = maxY - minY;
  const float depth  = maxZ - minZ;
  mesh->radius =
      std::sqrt(width * width + height * height + depth * depth) * 0.5f;
  mesh->center[0] = (minX + maxX) * 0.5f;
  mesh->center[1] = (minY + maxY) * 0.5f;
  mesh->center[2] = (minZ + maxZ) * 0.5f;

  glGenBuffers(1, &mesh->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(mesh->numVertices * sizeof(float)),
               mesh->vertices, GL_STATIC_DRAW);
  glGenBuffers(1, &mesh->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(mesh->numIndices * sizeof(unsigned int)),
               mesh->indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  Entry entry;
  entry.mesh     = mesh;
  entry.refCount = 1;
  _meshes[key]   = entry;
  OkLogger::info(
      "Mesh", "Shared mesh '" + key + "': " + std::to_string(mesh->numIndices) +
                  " indices, " + std::to_string(_meshes.size()) + " held");
  return mesh;
}

void OkMeshHandler::removeReference(const std::string &key) {
  std::map<std::string, Entry>::iterator found = _meshes.find(key);
  if (found == _meshes.end()) {
    return;
  }
  found->second.refCount--;
  if (found->second.refCount > 0) {
    return;
  }
  delete found->second.mesh;
  _meshes.erase(found);
}
