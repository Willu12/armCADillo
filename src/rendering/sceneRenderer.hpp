#pragma once
#include "IEntity.hpp"
#include "IEntityRenderer.hpp"
#include "IRenderable.hpp"
#include "bezierCurveRenderer.hpp"
#include "camera.hpp"
#include "centerPointRenderer.hpp"
#include "cursor.hpp"
#include "cursorRenderer.hpp"
#include "entitiesTypes.hpp"
#include "grid.hpp"
#include "pickingRenderer.hpp"
#include "pickingTexture.hpp"
#include "pointRenderer.hpp"
#include "torusRenderer.hpp"
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <vector>

#include "GLFW/glfw3.h"
#include "vec.hpp"

class SceneRenderer {
public:
  SceneRenderer(std::shared_ptr<Camera> camera, PickingTexture &pickingTexture,
                GLFWwindow *window)
      : _centerPointRenderer(*camera), _pickingRenderer(pickingTexture),
        _camera(camera), _window(window) {
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
  renderPicking(const std::vector<std::shared_ptr<IEntity>> &pickableEntities) {
    _pickingRenderer.render(pickableEntities, *_camera);
  }
  void renderCursor(const std::shared_ptr<Cursor> &cursor) {
    auto &cursorRenderer = _entityRenderers.at(EntityType::Cursor);
    cursorRenderer->render({cursor});
  }

  void renderCenterPoint(const IRenderable &centerPoint) {
    _centerPointRenderer.render(centerPoint);
  }

  void renderVirtualPoints(
      const std::vector<std::shared_ptr<IEntity>> &virtualPoints) {
    auto &virtualPointRenderer = _entityRenderers.at(EntityType::VirtualPoint);
    virtualPointRenderer->render(virtualPoints);
  }

private:
  std::unordered_map<EntityType, std::unique_ptr<IEntityRenderer>>
      _entityRenderers;

  CenterPointRenderer _centerPointRenderer;
  PickingRenderer _pickingRenderer;
  std::shared_ptr<Camera> _camera;
  GLFWwindow *_window;
  Grid grid;

  void initEntityRenderers() {
    _entityRenderers.insert(
        {EntityType::Torus, std::make_unique<TorusRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::Point,
         std::make_unique<PointRenderer>(
             *_camera, algebra::Vec4f{0.5f, 0.0f, 0.7f, 1.0f})});
    _entityRenderers.insert(
        {EntityType::VirtualPoint,
         std::make_unique<PointRenderer>(
             *_camera, algebra::Vec4f{0.0f, 1.0f, 0.0f, 1.0f})});
    _entityRenderers.insert(
        {EntityType::Cursor, std::make_unique<CursorRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::BezierCurveC0,
         std::make_unique<BezierCurveRenderer>(*_camera, _window)});
    _entityRenderers.insert(
        {EntityType::BezierCurveC2,
         std::make_unique<BezierCurveRenderer>(*_camera, _window)});
  }
};