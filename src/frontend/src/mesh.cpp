#include "mesh.hpp"

void Mesh::draw() {
  _shader.use();
  glBindVertexArray(VAO);

  printf("indices: %d\n", _indices.size());
  glDrawElements(GL_LINES, _indices.size(), GL_UNSIGNED_INT, 0);
}