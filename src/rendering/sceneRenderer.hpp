#pragma once
#include "IEntity.hpp"
#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "cursor.hpp"
#include "cursorRenderer.hpp"
#include "entitiesTypes.hpp"
#include "grid.hpp"
#include "pickingRenderer.hpp"
#include "pickingTexture.hpp"
#include "pointRenderer.hpp"
#include "torusRenderer.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class SceneRenderer {
public:
  SceneRenderer(std::shared_ptr<Camera> camera, PickingTexture &pickingTexture)
      : _pickingRenderer(pickingTexture), _camera(camera) {
    initEntityRenderers();
  }

  void
  render(const std::unordered_map<
         EntityType, std::vector<std::shared_ptr<IEntity>>> &groupedEntities) {
    grid.render(_camera);

    for (const auto &entityGroup : groupedEntities) {
      auto &renderer = _entityRenderers.at(entityGroup.first);
      renderer->render(entityGroup.second);
    }
  }

  void
  renderPicking(const std::vector<std::shared_ptr<IEntity>> pickableEntities) {
    _pickingRenderer.render(pickableEntities, _camera);
  }
  void renderCursor(const std::shared_ptr<Cursor> &cursor) {
    auto &cursorRenderer = _entityRenderers.at(EntityType::Cursor);
    cursorRenderer->render({cursor});
  }

private:
  std::unordered_map<EntityType, std::unique_ptr<IEntityRenderer>>
      _entityRenderers;
  PickingRenderer _pickingRenderer;
  std::shared_ptr<Camera> _camera;
  Grid grid;

  void initEntityRenderers() {
    _entityRenderers.insert(
        {EntityType::Torus, std::make_unique<TorusRenderer>(_camera)});
    _entityRenderers.insert(
        {EntityType::Point, std::make_unique<PointRenderer>(_camera)});
    _entityRenderers.insert(
        {EntityType::Cursor, std::make_unique<CursorRenderer>(_camera)});
  }
};