#pragma once
#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "torusModel.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera, TorusModel *torusModel, GLFWwindow *window)
      : _camera(camera), _torusModel(torusModel), _window(window) {}

  void renderMesh(const Mesh &mesh, Shader &shader) {
    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(_torusModel->getModelMatrix());

    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    shader.setProjectionMatrix(aspectRatio);
    shader.use();
    glBindVertexArray(mesh.VAO);

    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

private:
  Camera *_camera;
  TorusModel *_torusModel;
  GLFWwindow *_window;
};