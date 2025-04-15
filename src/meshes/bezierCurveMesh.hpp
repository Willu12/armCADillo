#pragma once

#include "IMeshable.hpp"
#include "glad/glad.h"
#include "vec.hpp"
#include <cstdio>
#include <memory>

struct BezierVertex {
  int len;
  algebra::Vec3f pts[4];
};

class BezierMesh : public IMeshable {
public:
  uint32_t getVAO() const override { return _vao; }
  uint32_t getVBO() const override { return _vbo; }
  uint32_t getEBO() const override { return _ebo; }
  uint32_t getIndicesLength() const override { return _bezierSegments.size(); }

  static std::unique_ptr<BezierMesh>
  create(const std::vector<algebra::Vec3f> &vertices) {
    auto *mesh = new BezierMesh(vertices);
    mesh->addSimpleVertexLayout();
    return std::unique_ptr<BezierMesh>(mesh);
  }

  ~BezierMesh() {
    if (_vao > 0)
      glDeleteVertexArrays(1, &_vao);
    if (_vbo > 0)
      glDeleteBuffers(1, &_vbo);
    if (_ebo > 0)
      glDeleteBuffers(1, &_ebo);

    _vao = _vbo = _ebo = 0;
  }

  BezierMesh(BezierMesh &&other) noexcept
      : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
        _bezierSegments(std::move(other._bezierSegments)) {
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
  }

  BezierMesh &operator=(BezierMesh &&other) noexcept {
    if (this != &other) {
      glDeleteVertexArrays(1, &_vao);
      glDeleteBuffers(1, &_vbo);
      glDeleteBuffers(1, &_ebo);

      _vao = other._vao;
      _vbo = other._vbo;
      _ebo = other._ebo;
      _bezierSegments = std::move(other._bezierSegments);

      other._vao = 0;
      other._vbo = 0;
      other._ebo = 0;
    }
    return *this;
  }

  BezierMesh(const BezierMesh &) = delete;
  BezierMesh &operator=(const BezierMesh &) = delete;

protected:
  BezierMesh(const std::vector<algebra::Vec3f> &vertices)
      : _bezierSegments(buildBezierVertexData(vertices)) {
    initBuffers();
  }

private:
  uint32_t _vao, _vbo, _ebo;
  std::vector<BezierVertex> _bezierSegments;

  void addSimpleVertexLayout() {
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(0, 1, GL_INT, sizeof(BezierVertex), (void *)0);

    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(1 + i);
      glVertexAttribPointer(1 + i,
                            3,                    // 3 components (x,y,z)
                            GL_FLOAT,             // Type
                            GL_FALSE,             // Normalized
                            sizeof(BezierVertex), // Stride
                            (void *)(sizeof(int) + sizeof(float) * i * 3));
    }
    glBindVertexArray(0);
  }

  void initBuffers() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _bezierSegments.size() * sizeof(BezierVertex),
                 _bezierSegments.data(), GL_STATIC_DRAW);
  }

  std::vector<BezierVertex>
  buildBezierVertexData(const std::vector<algebra::Vec3f> &points) {
    std::vector<BezierVertex> data;

    for (size_t i = 0; i < points.size();) {
      BezierVertex v;
      v.len = std::min((int)(points.size() - i), 4);
      for (int j = 0; j < 4; ++j) {
        v.pts[j] = (i + j < points.size()) ? points[i + j] : algebra::Vec3f();
      }
      data.push_back(v);
      i += 3;
    }
    return data;
  }
};