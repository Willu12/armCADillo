#include "mesh.hpp"

void Mesh::draw() {
  glBindVertexArray(VAO);
  _shader.use();

  glDrawElements(GL_LINES, _indices.size(), GL_UNSIGNED_INT, 0);
}