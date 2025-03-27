#pragma once
#include "shader.hpp"
#include "vec.hpp"
#include <vector>

class Mesh {
public:
  unsigned int VAO, VBO, EBO;
  std::vector<float> _vertices;
  std::vector<unsigned int> _indices;

  Mesh(const std::vector<float> &vertices,
       const std::vector<unsigned int> &_indices)
      : _vertices(vertices), _indices(_indices) {
    initBuffers();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  ~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

  Mesh(Mesh &&other) noexcept
      : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
        _vertices(std::move(other._vertices)),
        _indices(std::move(other._indices)) {
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
  }

  Mesh &operator=(Mesh &&other) noexcept {
    if (this != &other) {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);

      VAO = other.VAO;
      VBO = other.VBO;
      EBO = other.EBO;
      _vertices = std::move(other._vertices);
      _indices = std::move(other._indices);

      other.VAO = 0;
      other.VBO = 0;
      other.EBO = 0;
    }
    return *this;
  }

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

private:
  void addTextureLayout() {
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void initBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float),
                 _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 _indices.size() * sizeof(unsigned int), _indices.data(),
                 GL_STATIC_DRAW);
  }
};