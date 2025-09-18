#include "bezierSurfaceMesh.hpp"

#include "glad/gl.h"

#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

uint32_t BezierSurfaceMesh::getVAO() const { return _vao; }
uint32_t BezierSurfaceMesh::getVBO() const { return _vbo; }
uint32_t BezierSurfaceMesh::getEBO() const { return _ebo; }
uint32_t BezierSurfaceMesh::getIndicesLength() const {
  return _controlPoints.size();
}

std::unique_ptr<BezierSurfaceMesh>
BezierSurfaceMesh::create(const std::vector<float> &points, uint32_t u_patches,
                          uint32_t v_patches) {
  auto vertices = createC0MeshData(points, u_patches, v_patches);

  auto *bezierSurfaceMesh =
      new BezierSurfaceMesh(vertices, u_patches, v_patches);
  return std::unique_ptr<BezierSurfaceMesh>(bezierSurfaceMesh);
}
std::unique_ptr<BezierSurfaceMesh>
BezierSurfaceMesh::createC2(const std::vector<float> &points,
                            uint32_t u_patches, uint32_t v_patches) {

  auto *bezierSurfaceMesh = new BezierSurfaceMesh(points, u_patches, v_patches);
  return std::unique_ptr<BezierSurfaceMesh>(bezierSurfaceMesh);
}
BezierSurfaceMesh::~BezierSurfaceMesh() {
  if (_vao > 0)
    glDeleteVertexArrays(1, &_vao);
  if (_vbo > 0)
    glDeleteBuffers(1, &_vbo);
  if (_ebo > 0)
    glDeleteBuffers(1, &_ebo);

  _vao = _vbo = _ebo = 0;
}

BezierSurfaceMesh::BezierSurfaceMesh(BezierSurfaceMesh &&other) noexcept
    : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
      _controlPoints(std::move(other._controlPoints)) {
  other._vao = 0;
  other._vbo = 0;
  other._ebo = 0;
}

BezierSurfaceMesh &
BezierSurfaceMesh::operator=(BezierSurfaceMesh &&other) noexcept {
  if (this != &other) {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);

    _vao = other._vao;
    _vbo = other._vbo;
    _ebo = other._ebo;
    _controlPoints = std::move(other._controlPoints);
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
  }
  return *this;
}

BezierSurfaceMesh::BezierSurfaceMesh(const std::vector<float> &vertices,
                                     uint32_t u_patches, uint32_t v_patches)
    : _controlPoints(vertices) {
  initBuffers();
}

void BezierSurfaceMesh::initBuffers() {
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glGenBuffers(1, &_ebo);

  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, _controlPoints.size() * sizeof(float),
               _controlPoints.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

std::vector<float>
BezierSurfaceMesh::createC0MeshData(const std::vector<float> &vertices,
                                    uint32_t u_patches, uint32_t v_patches) {
  std::vector<float> meshData;

  const uint32_t u_points = 3 * u_patches + 1;
  for (uint32_t v_idx = 0; v_idx < v_patches; ++v_idx) {
    for (uint32_t u_idx = 0; u_idx < u_patches; ++u_idx) {
      for (uint32_t i = 0; i < 4; ++i) {
        for (uint32_t j = 0; j < 4; ++j) {
          uint32_t u = u_idx * 3 + i;
          uint32_t v = v_idx * 3 + j;
          uint32_t idx = 3 * (v * u_points + u);

          meshData.insert(meshData.end(), vertices.begin() + idx,
                          vertices.begin() + idx + 3);
        }
      }
    }
  }

  return meshData;
}
