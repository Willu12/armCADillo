#pragma once

#include "IRenderable.hpp"
#include "camera.hpp"
#include "shader.hpp"
class CenterPointRenderer {
public:
  CenterPointRenderer(const Camera &camera)
      : _camera(camera), _shader("../resources/shaders/centerPointShader.vs",
                                 "../resources/shaders/fragmentShader.hlsl") {}

  void render(const IRenderable &renderable) {
    _shader.use();

    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setModelMatrix(renderable.getModelMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());

    const auto &mesh = renderable.getMesh();
    glBindVertexArray(mesh.getVAO());
    glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }

private:
  const Camera &_camera;
  Shader _shader;
};