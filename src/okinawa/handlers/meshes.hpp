#ifndef OK_MESHES_HPP
#define OK_MESHES_HPP

#include "../core/gl_config.hpp"
#include <array>
#include <map>
#include <string>

/**
 * @file
 * @brief One upload of a mesh, drawn by any number of items.
 *
 * An item owns its geometry: it keeps a copy of the vertices in memory
 * and uploads buffers of its own. That is right for a mesh drawn once,
 * and wrong for one drawn in a thousand places -- a street lamp, a
 * window, a doorway. The same drawing then arrives once per group that
 * draws it, in memory AND on the card, and each of those uploads is
 * rounded up to a page whether it needs one or not.
 *
 * Measured on a city of a hundred and twenty thousand accessories: 12512
 * groups holding 75 distinct meshes between them, and 800 MB of memory
 * for what is 6 MB of geometry.
 *
 * So the geometry gets a life of its own, keyed by a name the caller
 * chooses -- the piece's own name, since two callers asking for the same
 * piece mean the same triangles. What stays with the item is everything
 * that is about this drawer rather than about the drawing: its transform,
 * its materials, its tints, and its own vertex array object, which is
 * where an instanced item hangs the buffer of where its copies stand.
 */

/** @brief The vertices and indices of one mesh, uploaded once. */
class OkSharedMesh {
public:
  /** @brief Vertices in the item layout (stride 8), and the triangles. */
  float        *vertices    = nullptr;
  unsigned int *indices     = nullptr;
  long          numVertices = 0;  // floats, not vertices
  long          numIndices  = 0;

  /** @brief The buffers on the card, shared by every item drawing this. */
  GLuint vbo = 0;
  GLuint ebo = 0;

  /** @brief The bounding sphere of the mesh, in its own coordinates. */
  float                radius = 0.0f;
  std::array<float, 3> center = {0.0f, 0.0f, 0.0f};

  ~OkSharedMesh();
};

/**
 * @brief The meshes shared between items, by name.
 *
 * Reference counted like the textures, and for the same reason: what
 * decides when geometry can go is the last drawer of it letting go, not
 * whoever happened to load it first.
 */
class OkMeshHandler {
public:
  OkMeshHandler(const OkMeshHandler &)            = delete;
  OkMeshHandler &operator=(const OkMeshHandler &) = delete;

  static OkMeshHandler *getInstance();

  /**
   * @brief The mesh under this name, uploading it the first time.
   *
   * @param key          what the mesh is called; the same name must mean
   *                     the same triangles, which is why it is the
   *                     piece's own name and not the item's.
   * @param vertexData   the caller's vertices, `vertexStride` floats each.
   * @param vertexCount  how many floats, not how many vertices.
   * @param indexData    its triangles, indices from zero.
   * @param indexCount   how many of them.
   * @param vertexStride 5 for x,y,z,u,v (normals worked out here), 8 for
   *                     vertices that carry their own.
   * @return the shared mesh, with this caller counted as a user of it,
   *         or null when there is nothing to upload.
   */
  OkSharedMesh *acquire(const std::string &key, const float *vertexData,
                        long vertexCount, const unsigned int *indexData,
                        long indexCount, int vertexStride);

  /** @brief One drawer of this mesh is gone; free it when the last is. */
  void removeReference(const std::string &key);

  /** @brief How many distinct meshes are being held, for a report. */
  size_t count() const {
    return _meshes.size();
  }

  ~OkMeshHandler();

private:
  OkMeshHandler() = default;

  struct Entry {
    OkSharedMesh *mesh     = nullptr;
    int           refCount = 0;
  };

  std::map<std::string, Entry> _meshes;

  static OkMeshHandler *_instance;
};

#endif
