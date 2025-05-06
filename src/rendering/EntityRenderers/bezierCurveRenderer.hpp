#pragma once

#include "IEntityRenderer.hpp"
#include "bezierCurveC0.hpp"
#include "camera.hpp"
#include "glfwHelper.hpp"
#include "mesh.hpp"
#include "shader.hpp"

#include "GLFW/glfw3.h"

class BezierCurveRenderer : public IEntityRenderer {
public:
  BezierCurveRenderer(const Camera &camera, GLFWwindow *window)
      : _camera(camera), _window(window),
        _shader("../resources/shaders/vertexBezier.vs",
                "../resources/shaders/geometryBezier.gs",
                "../resources/shaders/fragmentShader.hlsl") {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());
    _shader.setInt("screenResolution", 0.5 * (GLFWHelper::getHeight(_window) +
                                              GLFWHelper::getWidth(_window)));

    for (const auto &entity : entities) {
      auto &bezier = dynamic_cast<BezierCurve &>(*entity);
      _shader.setInt("renderPolyLine", bezier.showPolyLine());
      auto &mesh = entity->getMesh();
      glPointSize(10.0f);
      glBindVertexArray(mesh.getVAO());
      glDrawArrays(GL_POINTS, 0, mesh.getIndicesLength());
      glBindVertexArray(0);
    }
  }

private:
  const Camera &_camera;
  GLFWwindow *_window;
  Shader _shader;
};