#pragma once
#include "GLFW/glfw3.h"
#include "IRenderable.hpp"
#include "camera.hpp"
#include "glfwHelper.hpp"
#include "pickingTexture.hpp"
#include "torusEntity.hpp"
#include "transformations.hpp"

class MeshRenderer {
public:
  MeshRenderer(Camera *camera, GLFWwindow *window)
      : _camera(camera), _window(window) {}

  void renderEntities(const std::vector<IEntity *> &entites, Shader &shader) {
    for (int i = 0; i < entites.size(); ++i) {
      // printf("[%u], [%ld]\n", i + 1, entites.size());

      renderMesh(*entites[i], shader);
    }
  }

  void renderMesh(const IRenderable &entity, Shader &shader) {
    shader.use();

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(entity.getModelMatrix());
    shader.setProjectionMatrix(_camera->projectionMatrix());

    const Mesh &mesh = entity.getMesh();

    glBindVertexArray(mesh._vao);
    glDrawElements(GL_LINES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

  void renderBillboard(const IRenderable &entity, Shader &shader) {
    shader.use();

    auto entityWorldPos =
        _camera->viewMatrix() * entity.getPosition().toHomogenous();
    const float distanceFromCamera = std::abs(entityWorldPos[2]);
    auto scaleMatrix = algebra::transformations::scaleMatrix(
        distanceFromCamera, distanceFromCamera, distanceFromCamera);

    shader.setViewMatrix(_camera->viewMatrix());
    shader.setModelMatrix(
        entity.getModelMatrix() * scaleMatrix *
        _camera->getSphericalPosition().getRotationMatrix().transpose());
    shader.setProjectionMatrix(_camera->projectionMatrix());

    const Mesh &mesh = entity.getMesh();
    glBindVertexArray(mesh._vao);
    glDrawElements(GL_TRIANGLES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  }

  void renderPicking(const std::vector<IEntity *> &entites, Shader &shader,
                     PickingTexture &pickingTexture) {

    pickingTexture.enableWriting();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (uint32_t i = 0; i < entites.size(); ++i) {
      shader.use();
      shader.setUInt("gObjectIndex", i + 1);
      renderMesh(*entites[i], shader);
    }
    pickingTexture.disableWrtiing();
  }

private:
  Camera *_camera;
  GLFWwindow *_window;
};