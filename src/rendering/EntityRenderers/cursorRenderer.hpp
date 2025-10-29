#pragma once

#include "IEntity.hpp"
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <memory>

class CursorRenderer : public IEntityRenderer {
public:
  explicit CursorRenderer(const Camera &camera)
      : _camera(camera),
        _shader("../resources/shaders/textureShader.vert",
                "../resources/shaders/texturedBillboardShader.frag"),
        _texture(
            Texture::createTexture("../resources/textures/cursorTexture.png")) {
  }

  void render(const std::vector<IEntity *> &entities) override {
    _texture->bind(0);
    _shader.use();
    for (const auto *entity : entities) {
      const float camera_distance = distanceFromCamera(entity);
      auto scale_matrix = algebra::transformations::scaleMatrix(
          camera_distance, camera_distance, camera_distance);

      _shader.setViewMatrix(_camera.viewMatrix());
      _shader.setModelMatrix(
          entity->getModelMatrix() * scale_matrix *
          _camera.getSphericalPosition().getRotationMatrix().transpose());
      _shader.setProjectionMatrix(_camera.getProjectionMatrix());

      const auto &mesh = entity->getMesh();

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBindVertexArray(mesh.getVAO());
      glDrawElements(GL_TRIANGLES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);

      glBindVertexArray(0);
      glDisable(GL_BLEND);
    }
  }

private:
  const Camera &_camera;
  Shader _shader;
  std::unique_ptr<Texture> _texture;

  float distanceFromCamera(const IEntity *entity) {
    auto entity_world_pos =
        _camera.viewMatrix() * entity->getPosition().toHomogenous();
    return std::abs(entity_world_pos[2]);
  }
};