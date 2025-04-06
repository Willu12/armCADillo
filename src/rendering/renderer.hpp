#pragma once
#include "IEntity.hpp"
#include "IRenderable.hpp"
#include "camera.hpp"
#include "entitiesTypes.hpp"
#include "pickingTexture.hpp"
#include "shader.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class MeshRenderer {
public:
  MeshRenderer(std::shared_ptr<Camera> camera) : _camera(camera) {}

  void renderEntities(const std::vector<std::shared_ptr<IEntity>> &entites,
                      Shader &shader);
  void renderMesh(const IRenderable &entity, Shader &shader);
  void renderBillboard(const IRenderable &entity, Shader &shader);
  void renderPicking(const std::vector<std::shared_ptr<IEntity>> &entites,
                     Shader &shader, PickingTexture &pickingTexture);

  void renderGroupedEntites(
      const std::unordered_map<
          EntityType, std::vector<std::shared_ptr<IEntity>>> &groupedEntities,
      Shader &shader);

  void renderGroupedPicking(
      const std::unordered_map<
          EntityType, std::vector<std::shared_ptr<IEntity>>> &groupedEntities,
      Shader &shader, PickingTexture &pickingTexture);

private:
  std::shared_ptr<Camera> _camera;

  GLuint prepareInstacedModelMatrices(
      const std::vector<std::shared_ptr<IEntity>> &entities);
  GLuint preparePickingInstacedBuffers(
      const std::vector<std::shared_ptr<IEntity>> &entities);
  void
  renderInstacedEntities(const std::vector<std::shared_ptr<IEntity>> &entities,
                         Shader &shader);
  void
  renderInstacedPicking(const std::vector<std::shared_ptr<IEntity>> &entities,
                        Shader &shader, PickingTexture &pickingTexture);
};