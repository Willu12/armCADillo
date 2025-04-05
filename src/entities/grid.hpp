#pragma once
#include "shader.hpp"

#include "GLFW/glfw3.h"
#include "camera.hpp"
#include <memory>

class Grid {
public:
  Grid(GLFWwindow *window)
      : _window(window),
        _shader("../resources/shaders/gridVertexShader.hlsl",
                "../resources/shaders/gridFragmentShader.hlsl") {}

  void render(std::shared_ptr<Camera> camera) {
    _shader.use();
    _shader.setViewMatrix(camera->viewMatrix());
    _shader.setProjectionMatrix(camera->projectionMatrix());
    _shader.setVec3f("cameraWorldPos", camera->getPosition());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

private:
  GLFWwindow *_window;
  Shader _shader;

  float getAspectRatio() const {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    return static_cast<float>(width) / static_cast<float>(height);
  }
};