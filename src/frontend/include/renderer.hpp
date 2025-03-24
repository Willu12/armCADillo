#pragma once
#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "torusModel.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera, GLFWwindow *window)
      : _camera(camera), _window(window) {}

  void renderMesh(const IEntity &entity, Shader &shader) {
    shader.use();

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(entity.getModelMatrix());
    shader.setProjectionMatrix(getAspectRatio());

    const Mesh &mesh = entity.getMesh();

    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

private:
  Camera *_camera;
  GLFWwindow *_window;

  float getAspectRatio() const {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    return static_cast<float>(width) / static_cast<float>(height);
  }
};