#pragma once
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "torusEntity.hpp"
#include <memory>

class TorusRenderer : public IEntityRenderer {
public:
  explicit TorusRenderer(const Camera &camera)
      : _shader("../resources/shaders/torusShader.vs",
                "../resources/shaders/fragmentShaderTrimmed.hlsl"),
        _camera(camera) {}

  void render(const std::vector<IEntity *> &entities) override {
    if (entities.empty()) {
      return;
    }
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());

    for (const auto &entity : entities) {
      const auto &mesh = entity->getMesh();
      auto &torus = dynamic_cast<TorusEntity &>(*entity);
      _shader.setVec4f("color", entity->getColor().toVector());

      if (torus.isTrimmed()) {
        _shader.setInt("trim", 1);
      } else {
        _shader.setInt("trim", 0);
      }
      if (torus.hasIntersectionTexture()) {
        const auto &texture = torus.getIntersectionTexutre();
        texture.bind();
      }
      glBindVertexArray(mesh.getVAO());
      _shader.setModelMatrix(entity->getModelMatrix());
      glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }
  }

private:
  Shader _shader;
  const Camera &_camera;
};