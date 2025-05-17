#pragma once

#include "IMeshable.hpp"
#include "glad/gl.h"
#include "vec.hpp"
#include <memory>
#include <vector>

class BezierSurfaceMesh : public IMeshable {
public:
  uint32_t getVAO() const override { return _vao; }
  uint32_t getVBO() const override { return _vbo; }
  uint32_t getEBO() const override { return _ebo; }
  uint32_t getIndicesLength() const override { return _controlPoints.size(); }

  static std::unique_ptr<BezierSurfaceMesh>
  create(const std::vector<float> &points) {
    auto *bezierSurfaceMesh = new BezierSurfaceMesh(points);
    return std::unique_ptr<BezierSurfaceMesh>(bezierSurfaceMesh);
  }
  ~BezierSurfaceMesh() {
    if (_vao > 0)
      glDeleteVertexArrays(1, &_vao);
    if (_vbo > 0)
      glDeleteBuffers(1, &_vbo);
    if (_ebo > 0)
      glDeleteBuffers(1, &_ebo);

    _vao = _vbo = _ebo = 0;
  }

  BezierSurfaceMesh(BezierSurfaceMesh &&other) noexcept
      : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
        _controlPoints(std::move(other._controlPoints)) {
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
  }

  BezierSurfaceMesh &operator=(BezierSurfaceMesh &&other) noexcept {
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

  BezierSurfaceMesh(const BezierSurfaceMesh &) = delete;
  BezierSurfaceMesh &operator=(const BezierSurfaceMesh &) = delete;

protected:
  explicit BezierSurfaceMesh(const std::vector<float> &vertices)
      : _controlPoints(vertices) {
    initBuffers();
  }

private:
  uint32_t _vao, _vbo, _ebo; // these may need to be changed idk why?
  std::vector<float> _controlPoints;
  void initBuffers() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _controlPoints.size() * sizeof(float),
                 _controlPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
  }
};