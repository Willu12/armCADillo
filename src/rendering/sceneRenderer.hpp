#pragma once
#include "IEntity.hpp"
#include "IEntityRenderer.hpp"
#include "IRenderable.hpp"
#include "bezierCurveRenderer.hpp"
#include "bezierSurfaceRenderer.hpp"
#include "camera.hpp"
#include "centerPointRenderer.hpp"
#include "cursor.hpp"
#include "cursorRenderer.hpp"
#include "entitiesTypes.hpp"
#include "gregorySurfaceRenderer.hpp"
#include "grid.hpp"
#include "imgui.h"
#include "pickingRenderer.hpp"
#include "pickingTexture.hpp"
#include "pointRenderer.hpp"
#include "polylineRenderer.hpp"
#include "selectionBoxRenderer.hpp"
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
        _selectedPointsRenderer(*camera,
                                algebra::Vec4f{0.0f, 0.9f, 0.9f, 1.0f}),
        _camera(camera), _window(window) {
    initEntityRenderers();
  }

  void
  render(const std::unordered_map<
         EntityType, std::vector<std::shared_ptr<IEntity>>> &groupedEntities) {
    _camera->updateProjectionMatrix(_camera->projectionMatrix());
    _grid.render(_camera);
    for (const auto &entityGroup : groupedEntities) {
      auto &renderer = _entityRenderers.at(entityGroup.first);
      renderer->render(entityGroup.second);
    }
  }

  void stereoscopicRender(
      const std::unordered_map<
          EntityType, std::vector<std::shared_ptr<IEntity>>> &groupedEntities) {
    _grid.render(_camera);
    _camera->updateProjectionMatrix(_camera->leftEyeProjectionMatrix());
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);

    for (const auto &entityGroup : groupedEntities) {
      auto &renderer = _entityRenderers.at(entityGroup.first);
      renderer->render(entityGroup.second);
    }
    glClear(GL_DEPTH_BUFFER_BIT);
    _camera->updateProjectionMatrix(_camera->RightEyeProjectionMatrix());

    glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE);
    for (const auto &entityGroup : groupedEntities) {
      auto &renderer = _entityRenderers.at(entityGroup.first);
      renderer->render(entityGroup.second);
    }
    _camera->updateProjectionMatrix(_camera->projectionMatrix());
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
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

  void renderSelectionBox(const Mouse &mouse) {
    if (mouse._isSelectionBoxActive)
      _selectionBoxRenderer.render(*_camera, mouse, _window);
  }

  void renderSelectedPoints(
      const std::vector<std::shared_ptr<IEntity>> &selectedPoints) {
    _selectedPointsRenderer.render(selectedPoints);
  }

private:
  std::unordered_map<EntityType, std::unique_ptr<IEntityRenderer>>
      _entityRenderers;

  CenterPointRenderer _centerPointRenderer;
  PickingRenderer _pickingRenderer;
  SelectionBoxRenderer _selectionBoxRenderer;
  PointRenderer _selectedPointsRenderer;
  std::shared_ptr<Camera> _camera;
  GLFWwindow *_window;
  Grid _grid;

  void initEntityRenderers() {
    _entityRenderers.insert(
        {EntityType::Torus, std::make_unique<TorusRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::Point,
         std::make_unique<PointRenderer>(
             *_camera, algebra::Vec4f{0.2f, 0.5f, 0.5f, 1.0f})});
    _entityRenderers.insert(
        {EntityType::VirtualPoint,
         std::make_unique<PointRenderer>(
             *_camera, algebra::Vec4f{0.0f, 0.8f, 0.0f, 1.0f})});
    _entityRenderers.insert(
        {EntityType::Cursor, std::make_unique<CursorRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::BezierCurveC0,
         std::make_unique<BezierCurveRenderer>(*_camera, _window)});
    _entityRenderers.insert(
        {EntityType::BSplineCurve,
         std::make_unique<BezierCurveRenderer>(*_camera, _window)});
    _entityRenderers.insert(
        {EntityType::InterpolatingSplineCurve,
         std::make_unique<BezierCurveRenderer>(*_camera, _window)});
    _entityRenderers.insert(
        {EntityType::BezierSurfaceC0,
         std::make_unique<BezierSurfaceRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::BezierSurfaceC2,
         std::make_unique<BezierSurfaceRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::GregorySurface,
         std::make_unique<GregorySurfaceRenderer>(*_camera)});
    _entityRenderers.insert(
        {EntityType::Polyline, std::make_unique<PolylineRenderer>(*_camera)});
  }
};