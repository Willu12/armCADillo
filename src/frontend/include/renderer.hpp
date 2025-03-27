#pragma once
#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "glfwHelper.hpp"
#include "torusModel.hpp"
#include "transformations.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera, GLFWwindow *window)
      : _camera(camera), _window(window) {}

  void renderMesh(const IEntity &entity, Shader &shader) {
    shader.use();

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(entity.getModelMatrix());
    shader.setProjectionMatrix(_camera->projectionMatrix());

    const Mesh &mesh = entity.getMesh();

    glBindVertexArray(mesh._vao);
    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

  void renderBillboard(const IEntity &entity, Shader &shader) {
    shader.use();

    auto entityWorldPos =
        _camera->viewMatrix() * entity.getPosition().toHomogenous();
    const float distanceFromCamera = std::abs(entityWorldPos[2]);
    auto scaleMatrix = algebra::transformations::scaleMatrix(
        distanceFromCamera, distanceFromCamera, distanceFromCamera);

    /*
auto projection = algebra::transformations::projection(
 GLFWHelper::getAspectRatio(_window));
auto sceneCursorPosition =
 projection *
 (_camera->viewMatrix() * entity.getPosition().toHomogenous());

sceneCursorPosition = sceneCursorPosition * (1.0f / sceneCursorPosition[3]);
float z_ndc = sceneCursorPosition[2];
float z_screen =
 (z_ndc + 1.0f) * 0.5f; // Convert NDC [-1,1] to depth range [0,1]
auto scaleMatrix = algebra::transformations::scaleMatrix(
 1.f / z_ndc, 1.f / z_ndc, 1.f / z_ndc);
 */

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(
        entity.getModelMatrix() * scaleMatrix *
        _camera->getSphericalPosition().getRotationMatrix().transpose());
    shader.setProjectionMatrix(_camera->projectionMatrix());

    const Mesh &mesh = entity.getMesh();
    glBindVertexArray(mesh._vao);
    glDrawElements(GL_TRIANGLES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

private:
  Camera *_camera;
  GLFWwindow *_window;
};