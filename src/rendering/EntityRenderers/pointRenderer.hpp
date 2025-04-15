#pragma once
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include <memory>

class PointRenderer : public IEntityRenderer {
public:
  explicit PointRenderer(const Camera &camera)
      : _shader("../resources/shaders/vertexShader.hlsl",
                "../resources/shaders/fragmentShader.hlsl"),
        _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;

    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());

    const auto &sampleMesh = entities[0]->getMesh();
    glBindVertexArray(sampleMesh.getVAO());
    GLuint mbuffer = prepareInstacedModelMatrices(entities);

    glDrawElementsInstanced(GL_TRIANGLES, sampleMesh.getIndicesLength(),
                            GL_UNSIGNED_INT, 0, entities.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
    glBindVertexArray(0);

    glDeleteBuffers(1, &mbuffer);
  }

private:
  Shader _shader;
  const Camera &_camera;
};