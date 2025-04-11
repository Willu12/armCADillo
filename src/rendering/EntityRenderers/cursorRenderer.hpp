#pragma once

#include "IEntity.hpp"
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <memory>

class CursorRenderer : public IEntityRenderer {
public:
  CursorRenderer(const Camera &camera)
      : _camera(camera),
        _shader("../resources/shaders/textureShader.vert",
                "../resources/shaders/texturedBillboardShader.frag"),
        _texture(prepareTexture()) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) {
    _texture->bind(0);
    _shader.use();
    for (const auto &entity : entities) {
      const float cameraDistance = distanceFromCamera(entity);
      auto scaleMatrix = algebra::transformations::scaleMatrix(
          cameraDistance, cameraDistance, cameraDistance);

      _shader.setViewMatrix(_camera.viewMatrix());
      _shader.setModelMatrix(
          entity->getModelMatrix() * scaleMatrix *
          _camera.getSphericalPosition().getRotationMatrix().transpose());
      _shader.setProjectionMatrix(_camera.projectionMatrix());

      const Mesh &mesh = entity->getMesh();

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBindVertexArray(mesh._vao);
      glDrawElements(GL_TRIANGLES, mesh._indices.size(), GL_UNSIGNED_INT, 0);

      glBindVertexArray(0);
      glDisable(GL_BLEND);
    }
  }

private:
  const Camera &_camera;
  Shader _shader;
  std::unique_ptr<Texture> _texture;

  float distanceFromCamera(const std::shared_ptr<IEntity> &entity) {
    auto entityWorldPos =
        _camera.viewMatrix() * entity->getPosition().toHomogenous();
    return std::abs(entityWorldPos[2]);
  }

  std::unique_ptr<Texture> prepareTexture() {
    Image image("../resources/textures/cursorTexture.png");
    TextureResource textureResource(image);
    Texture *texture = new Texture(textureResource);
    return std::unique_ptr<Texture>(texture);
  }
};