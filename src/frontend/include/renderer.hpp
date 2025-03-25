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

  void renderBillboard(const IEntity &entity, Shader &shader) {
    shader.use();

    auto entityWorldPos =
        _camera->viewMatrix() * entity.getPosition().toHomogenous();
    const float distanceFromCamera = std::abs(entityWorldPos[2]);
    auto scaleMatrix = algebra::transformations::scaleMatrix(
        distanceFromCamera, distanceFromCamera, distanceFromCamera);

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(
        _camera->getSphericalPosition().getRotationMatrix().transpose() *
        entity.getModelMatrix() * scaleMatrix);
    shader.setProjectionMatrix(getAspectRatio());

    const Mesh &mesh = entity.getMesh();
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
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