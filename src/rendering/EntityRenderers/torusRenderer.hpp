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
    //_shader.setProjectionMatrix(_camera.projectionMatrix());

    for (auto &entity : entities) {
      const auto &mesh = entity->getMesh();
      glBindVertexArray(mesh.getVAO());
      _shader.setModelMatrix(entity->getModelMatrix());
      _shader.setProjectionMatrix(_camera.leftEyeProjectionMatrix());
      glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE); // Red only
      glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);
      glClear(GL_DEPTH_BUFFER_BIT);

      _shader.setProjectionMatrix(_camera.RightEyeProjectionMatrix());
      glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE); // Red only
      glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }

private:
  Shader _shader;
  const Camera &_camera;
};