#pragma once

#include "EntityFactories/IEntityFactory.hpp"
#include "GuiVisitor.hpp"
#include "IEntity.hpp"
#include "centerPoint.hpp"
#include "controllers.hpp"
#include "cursor.hpp"
#include "entitiesTypes.hpp"
#include "jsonDeserializer.hpp"
#include "jsonSerializer.hpp"
#include "mouse.hpp"
#include "optional"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "virtualPoint.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class BezierCurveC0;

enum class ControllerKind : uint8_t {
  Camera = 0,
  Model = 1,
  Cursor = 2,
  Selection = 3
};

class GUI {
public:
  GUI(GLFWwindow *window, std::shared_ptr<Scene> scene);
  IController &getController();

  std::vector<std::shared_ptr<IEntity>> getEntities() const;
  std::vector<std::reference_wrapper<PointEntity>> getPoints() const;
  std::vector<std::shared_ptr<IEntity>> getSelectedEntities() const;
  std::vector<std::shared_ptr<IEntity>> getSelectedPointsPointers() const;

  std::shared_ptr<Cursor> getCursor();

  std::optional<const IRenderable *> getCenterPoint();

  void displayGUI();

  const Mouse &getMouse();
  PickingTexture &getPickingTexture();

  void setVirtualPoints(
      const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints,
      const std::vector<std::reference_wrapper<const VirtualPoint>>
          &selectedVirtualPoints);
  std::vector<std::shared_ptr<IEntity>> getSelectedVirtualPoints() const;
  std::vector<std::shared_ptr<IEntity>> getVirtualPoints() const;
  void clearVirtualPoints();

private:
  GLFWwindow *_window;
  std::shared_ptr<Scene> _scene;
  std::vector<std::shared_ptr<IEntity>> _selectedEntities;
  std::vector<std::shared_ptr<VirtualPoint>> _selectedVirtualPoints;
  std::vector<std::shared_ptr<VirtualPoint>> _virtualPoints;
  std::vector<std::shared_ptr<IController>> _controllers;
  ControllerKind _selectedController = ControllerKind::Camera;
  CenterPoint _centerPoint;
  Mouse _mouse;
  GuiVisitor _guiSettingsVisitor;
  JsonSerializer _jsonSerializer;
  JsonDeserializer _jsonDeserializer;

  std::unordered_map<EntityType, std::shared_ptr<IEntityFactory>>
      _entityFactories;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initEnitityFactories();
  void initControllers();
  void processControllers();

  void renderModelControllSettings();
  void renderCursorControllerSettings();

  void renderCreateTorusUI();
  void renderCreatePointUI();
  void removeButtonUI();
  void createBezierCurveUI();
  void createBSplineCurveUI();
  void createInterpolatingSplineCurveUI();
  void createBezierSurfaceC0UI();
  void createBezierSurfaceC2UI();
  void createSerializeUI();
  void createLoadSceneUI();

  IEntity &createEntity(EntityType entityType);
  void createBezierCurve();
  void createBSplineCurve();
  void createInterpolatingSplineCurve();
  void createBezierSurfaceC0Flat(uint32_t uPatches, uint32_t vPatches);
  void createBezierSurfaceC2Flat(uint32_t uPatches, uint32_t vPatches);

  void createBezierSurfaceC0Cylinder(float r, float h);
  void createBezierSurfaceC2Cylinder(float r, float h);

  void renderModelSettings();

  void displayEntitiesList();
  void deleteSelectedEntities();
  void selectEntity(int entityIndex);
  void unselectEntity(int entityIndex);

  void calculateFPS();
  void showFPSCounter();

  std::shared_ptr<ModelController> getModelController();
  std::shared_ptr<SelectionController> getSelectionController();
  std::vector<std::shared_ptr<IController>> getActiveControllers();

  std::vector<std::reference_wrapper<PointEntity>> getSelectedPoints();
  std::vector<std::reference_wrapper<BezierCurve>> getSelectedBezierCurves();
};