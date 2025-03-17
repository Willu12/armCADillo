#include "mesh.hpp"

void Mesh::draw() {
  _shader.use();
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
}