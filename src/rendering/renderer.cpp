#include "renderer.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "pickingTexture.hpp"
#include "shader.hpp"

void MeshRenderer::renderEntities(
    const std::vector<std::shared_ptr<IEntity>> &entites, Shader &shader) {
  for (int i = 0; i < entites.size(); ++i) {
    renderMesh(*entites[i], shader);
  }
}

void MeshRenderer::renderMesh(const IRenderable &entity, Shader &shader) {
  shader.use();

  shader.setViewMatrix(_camera->viewMatrix());
  shader.setModelMatrix(entity.getModelMatrix());
  shader.setProjectionMatrix(_camera->projectionMatrix());

  const Mesh &mesh = entity.getMesh();
  glBindVertexArray(mesh._vao);

  auto meshKind =
      entity.getMeshKind() == MeshKind::Lines ? GL_LINES : GL_TRIANGLES;
  glDrawElements(meshKind, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void MeshRenderer::renderBillboard(const IRenderable &entity, Shader &shader) {
  shader.use();

  auto entityWorldPos =
      _camera->viewMatrix() * entity.getPosition().toHomogenous();
  const float distanceFromCamera = std::abs(entityWorldPos[2]);
  auto scaleMatrix = algebra::transformations::scaleMatrix(
      distanceFromCamera, distanceFromCamera, distanceFromCamera);

  shader.setViewMatrix(_camera->viewMatrix());
  shader.setModelMatrix(
      entity.getModelMatrix() * scaleMatrix *
      _camera->getSphericalPosition().getRotationMatrix().transpose());
  shader.setProjectionMatrix(_camera->projectionMatrix());

  const Mesh &mesh = entity.getMesh();
  glBindVertexArray(mesh._vao);
  glDrawElements(GL_TRIANGLES, mesh._indices.size(), GL_UNSIGNED_INT, 0);
  // glBindVertexArray(0);
}

void MeshRenderer::renderPicking(
    const std::vector<std::shared_ptr<IEntity>> &entites, Shader &shader,
    PickingTexture &pickingTexture) {

  pickingTexture.enableWriting();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (uint32_t i = 0; i < entites.size(); ++i) {
    shader.use();
    shader.setUInt("gObjectIndex", i + 1);
    renderMesh(*entites[i], shader);
  }

  pickingTexture.disableWriting();
}

void MeshRenderer::renderInstacedEntities(
    const std::vector<std::shared_ptr<IEntity>> &entities, Shader &shader) {
  if (entities.empty())
    return;

  shader.use();
  shader.setViewMatrix(_camera->viewMatrix());
  shader.setProjectionMatrix(_camera->projectionMatrix());

  const Mesh &sampleMesh = entities[0]->getMesh();
  glBindVertexArray(sampleMesh._vao);
  GLuint mbuffer = prepareInstacedModelMatrices(entities);

  auto meshKind =
      entities[0]->getMeshKind() == MeshKind::Lines ? GL_LINES : GL_TRIANGLES;
  glDrawElementsInstanced(meshKind, sampleMesh._indices.size(), GL_UNSIGNED_INT,
                          0, entities.size());
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
  glBindVertexArray(0);

  glDeleteBuffers(1, &mbuffer);
}

void MeshRenderer::renderInstacedPicking(
    const std::vector<std::shared_ptr<IEntity>> &entities, Shader &shader,
    PickingTexture &pickingTexture) {
  if (entities.empty())
    return;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader.use();
  shader.setViewMatrix(_camera->viewMatrix());
  shader.setProjectionMatrix(_camera->projectionMatrix());

  const Mesh &sampleMesh = entities[0]->getMesh();
  glBindVertexArray(sampleMesh._vao);
  auto mbuffer = preparePickingInstacedBuffers(entities);

  auto meshKind =
      entities[0]->getMeshKind() == MeshKind::Lines ? GL_LINES : GL_TRIANGLES;
  glDrawElementsInstanced(meshKind, sampleMesh._indices.size(), GL_UNSIGNED_INT,
                          0, entities.size());
  glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
  glBindVertexArray(0);
  glDeleteBuffers(1, &mbuffer);
}

void MeshRenderer::renderGroupedEntites(
    const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>>
        &groupedEntities,
    Shader &shader) {
  for (const auto &entitiesGroup : groupedEntities) {
    auto entities = entitiesGroup.second;
    renderInstacedEntities(entities, shader);
  }
}

void MeshRenderer::renderGroupedPicking(
    const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>>
        &groupedEntities,
    Shader &shader, PickingTexture &pickingTexture) {
  pickingTexture.enableWriting();

  for (const auto &entitiesGroup : groupedEntities) {
    auto entities = entitiesGroup.second;
    renderInstacedPicking(entities, shader, pickingTexture);
  }
  pickingTexture.disableWriting();
}

GLuint MeshRenderer::prepareInstacedModelMatrices(
    const std::vector<std::shared_ptr<IEntity>> &entities) {
  std::vector<algebra::Mat4f> modelMatrices;
  for (const auto &entity : entities) {
    modelMatrices.push_back(entity->getModelMatrix().transpose());
  }

  GLuint modelBuffer;
  glGenBuffers(1, &modelBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
  glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(algebra::Mat4f),
               modelMatrices.data(), GL_DYNAMIC_DRAW);

  for (int i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(1 + i);
    glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(algebra::Mat4f),
                          (void *)(sizeof(float) * i * 4));
    glVertexAttribDivisor(1 + i, 1);
  }
  return modelBuffer;
}

GLuint MeshRenderer::preparePickingInstacedBuffers(
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
