#pragma once
#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "shader.hpp"

class Grid {
public:
  Grid(GLFWwindow *window)
      : _window(window), _shader("../shaders/gridVertexShader.hlsl",
                                 "../shaders/gridFragmentShader.hlsl") {}

  void render(Camera *camera) {
    _shader.use();
    _shader.setViewMatrix(camera->viewMatrix());
    _shader.setProjectionMatrix(camera->projectionMatrix());
    _shader.setVec3f("cameraWorldPos", camera->getPosition());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

private:
  Shader _shader;
  GLFWwindow *_window;

  float getAspectRatio() const {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    return static_cast<float>(width) / static_cast<float>(height);
  }
};