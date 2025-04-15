#pragma once
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include <memory>

class TorusRenderer : public IEntityRenderer {
public:
  explicit TorusRenderer(const Camera &camera)
      : _shader("../resources/shaders/torusShader.vs",
                "../resources/shaders/fragmentShader.hlsl"),
        _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());

    for (auto &entity : entities) {
      const auto &mesh = entity->getMesh();
      _shader.setModelMatrix(entity->getModelMatrix());
      glBindVertexArray(mesh.getVAO());
      glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }
  }

private:
  Shader _shader;
  const Camera &_camera;
};