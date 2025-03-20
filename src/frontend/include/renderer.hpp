#pragma once
#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "torusModel.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera, TorusModel *torusModel, GLFWwindow *window)
      : _camera(camera), _torusModel(torusModel), _window(window) {}

  void renderMesh(const Mesh &mesh, Shader &shader) {
    shader.use();

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(_torusModel->getModelMatrix());
    shader.setProjectionMatrix(getAspectRatio());

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

private:
  Camera *_camera;
  TorusModel *_torusModel;
  GLFWwindow *_window;

  float getAspectRatio() const {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    return static_cast<float>(width) / static_cast<float>(height);
  }
};