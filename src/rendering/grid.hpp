#pragma once
#include "shader.hpp"

#include "camera.hpp"
#include <memory>

class Grid {
public:
  Grid()
      : _shader("../resources/shaders/gridVertexShader.hlsl",
                "../resources/shaders/gridFragmentShader.hlsl") {}

  void render(std::shared_ptr<Camera> camera) {
    _shader.use();
    _shader.setViewMatrix(camera->viewMatrix());
    _shader.setProjectionMatrix(camera->projectionMatrix());
    _shader.setVec3f("cameraWorldPos", camera->getPosition());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
  }

private:
  Shader _shader;
};