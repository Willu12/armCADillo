#pragma once
#include "IEntity.hpp"
#include "IRenderable.hpp"
#include "camera.hpp"
#include "pickingTexture.hpp"
#include "transformations.hpp"

class MeshRenderer {
public:
  MeshRenderer(std::shared_ptr<Camera> camera) : _camera(camera) {}

  void renderEntities(const std::vector<std::shared_ptr<IEntity>> &entites,
                      Shader &shader) {
    for (int i = 0; i < entites.size(); ++i) {
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
    auto meshKind =
        entity.getMeshKind() == MeshKind::Lines ? GL_LINES : GL_TRIANGLES;
    glDrawElements(meshKind, mesh._indices.size(), GL_UNSIGNED_INT, 0);
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

  void renderPicking(const std::vector<std::shared_ptr<IEntity>> &entites,
                     Shader &shader, PickingTexture &pickingTexture) {

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
  std::shared_ptr<Camera> _camera;
};