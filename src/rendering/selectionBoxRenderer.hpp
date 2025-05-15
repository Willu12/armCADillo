#pragma once

#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "glfwHelper.hpp"
#include "mouse.hpp"
#include "shader.hpp"
class SelectionBoxRenderer {
public:
  SelectionBoxRenderer()
      : _shader("../resources/shaders/selectionBoxVS.glsl",
                "../resources/shaders/selectionBoxFS.glsl") {}
  void render(const Camera &camera, const Mouse &mouse, GLFWwindow *window) {
    _shader.use();
    _shader.setViewMatrix(camera.viewMatrix());
    _shader.setProjectionMatrix(camera.projectionMatrix());

    auto startPos = mouse.getLastClickedPosition();
    auto endPos = mouse.getCurrentPosition();
    auto windowWidth = GLFWHelper::getWidth(window);
    auto windowHeight = GLFWHelper::getHeight(window);

    startPos[0] = 2.0f * startPos[0] / static_cast<float>(windowWidth) - 1.0f;
    startPos[1] = 1.0f - 2.0f * startPos[1] / static_cast<float>(windowHeight);

    endPos[0] = 2.0f * endPos[0] / static_cast<float>(windowWidth) - 1.0f;
    endPos[1] = 1.0f - 2.0f * endPos[1] / static_cast<float>(windowHeight);
    if (startPos[0] > endPos[0])
      std::swap(startPos[0], endPos[0]);
    if (startPos[1] > endPos[1])
      std::swap(startPos[1], endPos[1]);

    _shader.setVec2f("startPos", startPos);
    _shader.setVec2f("endPos", endPos);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
  }

private:
  Shader _shader;
  // std::we
};