#pragma once
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include <memory>

class TorusRenderer : public IEntityRenderer {
public:
  explicit TorusRenderer(const Camera &camera)
      : _shader("../resources/shaders/vertexShader.hlsl",
                "../resources/shaders/fragmentShader.hlsl"),
        _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;

    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());

    const Mesh &sampleMesh = entities[0]->getMesh();
    glBindVertexArray(sampleMesh._vao);
    GLuint mbuffer = prepareInstacedModelMatrices(entities);

    auto meshKind =
        entities[0]->getMeshKind() == MeshKind::Lines ? GL_LINES : GL_TRIANGLES;
    glDrawElementsInstanced(meshKind, sampleMesh._indices.size(),
                            GL_UNSIGNED_INT, 0, entities.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
    glBindVertexArray(0);

    glDeleteBuffers(1, &mbuffer);
  }

private:
  Shader _shader;
  const Camera &_camera;
};