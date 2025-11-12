#pragma once
#include "shader.hpp"

#include "camera.hpp"

class GridRenderer {
public:
  GridRenderer()
      : shader_("../../resources/shaders/grid.vert",
                "../../resources/shaders/grid.frag") {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindVertexArray(0);
  }

  void render(const Camera *camera) {
    shader_.use();
    shader_.setViewMatrix(camera->viewMatrix());
    shader_.setProjectionMatrix(camera->projectionMatrix());
    shader_.setVec3f("cameraWorldPos", camera->getPosition());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    /// set depth and blending
    glDisable(GL_DEPTH);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    /// clear depth and blending
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH);
    glDepthMask(GL_TRUE);
  }

private:
  Shader shader_;
  uint32_t vao_{};
};