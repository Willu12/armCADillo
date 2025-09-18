#include "gregoryMesh.hpp"
#include "glad/gl.h"
#include "gregoryQuad.hpp"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

uint32_t GregoryMesh::getVAO() const { return _vao; }
uint32_t GregoryMesh::getVBO() const { return _vbo; }
uint32_t GregoryMesh::getEBO() const { return _ebo; }
uint32_t GregoryMesh::getIndicesLength() const { return _vertices.size(); }

std::unique_ptr<GregoryMesh> GregoryMesh::create(const GregoryQuad &quad) {
  auto vertices = createMeshData(quad);
  auto *mesh = new GregoryMesh(vertices);
  return std::unique_ptr<GregoryMesh>(mesh);
}

GregoryMesh::~GregoryMesh() {
  if (_vao > 0)
    glDeleteVertexArrays(1, &_vao);
  if (_vbo > 0)
    glDeleteBuffers(1, &_vbo);
  if (_ebo > 0)
    glDeleteBuffers(1, &_ebo);

  _vao = _vbo = _ebo = 0;
}

GregoryMesh::GregoryMesh(GregoryMesh &&other) noexcept
    : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
      _vertices(std::move(other._vertices)) {
  other._vao = 0;
  other._vbo = 0;
  other._ebo = 0;
}

GregoryMesh &GregoryMesh::operator=(GregoryMesh &&other) noexcept {
  if (this != &other) {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);

    _vao = other._vao;
    _vbo = other._vbo;
    _ebo = other._ebo;
    _vertices = std::move(other._vertices);
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
  }
  return *this;
}

GregoryMesh::GregoryMesh(const std::vector<float> &vertices)
    : _vertices(vertices) {
  initBuffers();
}

void GregoryMesh::initBuffers() {
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glGenBuffers(1, &_ebo);

  glBindVertexArray(_vao);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float),
               _vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

std::vector<float> GregoryMesh::createMeshData(const GregoryQuad &quad) {
  std::vector<float> vertices(20 * 3);
  for (int i = 0; i < 4; ++i)
    for (int px = 0; px < 3; ++px)
      vertices[i * 3 + px] = quad.top[i][px];

  for (int i = 0; i < 2; ++i)
    for (int px = 0; px < 3; ++px)
      vertices[4 * 3 + i * 3 + px] = quad.topSides[i][px];

  for (int i = 0; i < 2; ++i)
    for (int px = 0; px < 3; ++px)
      vertices[6 * 3 + i * 3 + px] = quad.bottomSides[i][px];

  for (int i = 0; i < 4; ++i)
    for (int px = 0; px < 3; ++px)
      vertices[8 * 3 + i * 3 + px] = quad.bottom[i][px];

  for (int i = 0; i < 4; ++i)
    for (int px = 0; px < 3; ++px)
      vertices[12 * 3 + i * 3 + px] = quad.uInner[i][px];

  for (int i = 0; i < 4; ++i)
    for (int px = 0; px < 3; ++px)
      vertices[16 * 3 + i * 3 + px] = quad.vInner[i][px];

  return vertices;
}