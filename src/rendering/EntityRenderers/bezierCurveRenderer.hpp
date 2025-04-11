#pragma once

#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"

class BezierCurveRenderer : public IEntityRenderer {
public:
  BezierCurveRenderer(const Camera &camera)
      : _camera(camera), _shader("../resources/shaders/vertexBezier.vs",
                                 "../resources/shaders/geometryBezier.gs",
                                 "../resources/shaders/fragmentShader.hlsl") {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) {
    if (entities.empty())
      return;

    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());
    _shader.setInt("uNumSegments", 100); // FIX ME

    for (const auto &entity : entities) {
      auto &mesh = entity->getMesh();
      glBindVertexArray(mesh._vao);
      glDrawElements(GL_LINES_ADJACENCY, mesh._indices.size(), GL_UNSIGNED_INT,
                     0);
      glBindVertexArray(0);
    }
  }

private:
  const Camera &_camera;
  Shader _shader;
};