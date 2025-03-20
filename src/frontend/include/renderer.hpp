#pragma once
#include "camera.hpp"
#include "mesh.hpp"
#include "torusModel.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera, TorusModel *torusModel)
      : _camera(camera), _torusModel(torusModel) {}
  void renderMesh(const Mesh &mesh, Shader &shader) {
    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(_torusModel->getModelMatrix());
    shader.use();
    glBindVertexArray(mesh.VAO);

    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

private:
  Camera *_camera;
  TorusModel *_torusModel;
};