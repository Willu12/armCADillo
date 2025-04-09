#pragma once
#include "IEntity.hpp"
#include "camera.hpp"
#include "pickingTexture.hpp"
#include "shader.hpp"
#include <memory>

class PickingRenderer {
public:
  PickingRenderer(PickingTexture &pickingTexture)
      : _pickingTexture(pickingTexture),
        _shader("../resources/shaders/vertexPickingShader.hlsl",
                "../resources/shaders/pickingShader.frag") {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities,
              std::shared_ptr<Camera> camera) {

    if (entities.empty())
      return;
    _pickingTexture.enableWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _shader.use();
    _shader.setViewMatrix(camera->viewMatrix());
    _shader.setProjectionMatrix(camera->projectionMatrix());

    const Mesh &sampleMesh = entities[0]->getMesh();
    glBindVertexArray(sampleMesh._vao);
    auto mbuffer = preparePickingInstacedBuffers(entities);

    auto meshKind =
        entities[0]->getMeshKind() == MeshKind::Lines ? GL_LINES : GL_TRIANGLES;
    glDrawElementsInstanced(meshKind, sampleMesh._indices.size(),
                            GL_UNSIGNED_INT, 0, entities.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
    glBindVertexArray(0);
    glDeleteBuffers(1, &mbuffer);
    _pickingTexture.disableWriting();
  }

private:
  PickingTexture &_pickingTexture;
  Shader _shader;

  GLuint preparePickingInstacedBuffers(
      const std::vector<std::shared_ptr<IEntity>> &entities) {
    struct InstanceData {
      algebra::Mat4f modelMatrix;
      uint32_t objectIndex;
    };

    std::vector<InstanceData> instanceData;
    for (uint32_t i = 0; i < entities.size(); ++i) {
      InstanceData data;
      data.modelMatrix = entities[i]->getModelMatrix().transpose();
      data.objectIndex = i + 1;
      instanceData.push_back(data);
    }

    GLuint instanceBuffer;
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(InstanceData),
                 instanceData.data(), GL_DYNAMIC_DRAW);

    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(1 + i);
      glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                            (void *)(sizeof(float) * i * 4));
      glVertexAttribDivisor(1 + i, 1);
    }

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(InstanceData),
                           (void *)(sizeof(algebra::Mat4f)));
    glVertexAttribDivisor(5, 1);

    return instanceBuffer;
  }
};