#pragma once
#include "IParametrizable.hpp"
#include "shader.hpp"
#include "vec.hpp"
#include <vector>

struct MeshDensity {
  int s = 20;
  int t = 20;
};

class Mesh {
public:
  uint32_t _vao, _vbo, _ebo;
  std::vector<float> _vertices;
  std::vector<uint32_t> _indices;

  static std::shared_ptr<Mesh> create(const std::vector<float> &vertices,
                                      const std::vector<uint32_t> &indices) {
    auto *mesh = new Mesh(vertices, indices);
    mesh->addSimpleVertexLayout();
    return std::shared_ptr<Mesh>(mesh);
  }

  static std::shared_ptr<Mesh>
  createTexturedMesh(const std::vector<float> &vertices,
                     const std::vector<uint32_t> &indices) {
    auto *mesh = new Mesh(vertices, indices);
    mesh->addTextureLayout();
    return std::shared_ptr<Mesh>(mesh);
  }

  static std::shared_ptr<Mesh>
  fromParametrization(const IParametrizable<float> &parametrizable) {}

  ~Mesh() {
    if (_vao > 0)
      glDeleteVertexArrays(1, &_vao);
    if (_vbo > 0)
      glDeleteBuffers(1, &_vbo);
    if (_ebo > 0)
      glDeleteBuffers(1, &_ebo);

    _vao = _vbo = _ebo = 0;
  }

  Mesh(Mesh &&other) noexcept
      : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
        _vertices(std::move(other._vertices)),
        _indices(std::move(other._indices)) {
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
  }

  Mesh &operator=(Mesh &&other) noexcept {
    if (this != &other) {
      glDeleteVertexArrays(1, &_vao);
      glDeleteBuffers(1, &_vbo);
      glDeleteBuffers(1, &_ebo);

      _vao = other._vao;
      _vbo = other._vbo;
      _ebo = other._ebo;
      _vertices = std::move(other._vertices);
      _indices = std::move(other._indices);

      other._vao = 0;
      other._vbo = 0;
      other._ebo = 0;
    }
    return *this;
  }

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

protected:
  Mesh(const std::vector<float> &vertices,
       const std::vector<uint32_t> &_indices)
      : _vertices(vertices), _indices(_indices) {
    initBuffers();
  }

private:
  void addSimpleVertexLayout() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  void addTextureLayout() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void initBuffers() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float),
                 _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t),
                 _indices.data(), GL_STATIC_DRAW);
  }
};