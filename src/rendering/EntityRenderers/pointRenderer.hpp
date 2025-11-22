#pragma once
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "vec.hpp"

class PointRenderer : public IEntityRenderer {
public:
  explicit PointRenderer(const Camera &camera, algebra::Vec4f color)
      : _shader("../../resources/shaders/vertexShader.hlsl",
                "../../resources/shaders/colorFragmentShader.hlsl"),
        _camera(camera), _color(color) {}

  void render(const std::vector<IEntity *> &entities) override {
    if (entities.empty()) {
      return;
    }

    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());
    _shader.setVec4f("Color", _color);

    const auto &mesh = entities.front()->getMesh();
    glBindVertexArray(mesh.getVAO());
    GLuint mbuffer = prepareInstacedModelMatrices(entities);

    glDrawElementsInstanced(GL_TRIANGLES, mesh.getIndicesLength(),
                            GL_UNSIGNED_INT, nullptr, entities.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
    glBindVertexArray(0);

    glDeleteBuffers(1, &mbuffer);
  }

private:
  Shader _shader;
  const Camera &_camera;
  algebra::Vec4f _color{0.7f, 0.f, 0.7f, 1.f};
};