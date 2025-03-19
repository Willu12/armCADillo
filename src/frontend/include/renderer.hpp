#pragma once
#include "camera.hpp"
#include "mesh.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera) : _camera(camera) {}
  void renderMesh(const Mesh &mesh, Shader &shader) {
    shader.setViewMatrix(_camera->viewMatrix());
    shader.use();
    glBindVertexArray(mesh.VAO);

    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

private:
  Camera *_camera;
};