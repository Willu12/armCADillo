#pragma once

#include "IEntityRenderer.hpp"
#include "bezierCurve.hpp"
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

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());
    _shader.setInt("screenResolution", 0.5 * (GLFWHelper::getHeight(_window) +
                                              GLFWHelper::getWidth(_window)));

    for (const auto &entity : entities) {
      BezierCurve &bezier = static_cast<BezierCurve &>(*entity);
      _shader.setInt("renderPolyLine", bezier.showPolyLine());
      auto &mesh = entity->getMesh();

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