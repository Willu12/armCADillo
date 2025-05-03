#pragma once

#include "EntityFactories/IEntityFactory.hpp"
#include "GuiVisitor.hpp"
#include "IEntity.hpp"
#include "centerPoint.hpp"
#include "controllers.hpp"
#include "cursor.hpp"
#include "entitiesTypes.hpp"
#include "mouse.hpp"
#include "optional"
#include "pointEntity.hpp"
#include "scene.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class BezierCurveC0;

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2, Selection = 3 };
enum class ControllMode { Transformation = 0, Selection = 1 };

class GUI {
public:
  GUI(GLFWwindow *window, std::shared_ptr<Scene> scene);
  IController &getController();

  const std::vector<std::shared_ptr<IEntity>> getEntities() const;
  std::vector<std::reference_wrapper<const PointEntity>> getPoints() const;
  std::vector<std::shared_ptr<IEntity>> getSelectedEntities() const;

  std::shared_ptr<Cursor> getCursor();

  std::optional<const IRenderable *> getCenterPoint();

  void displayGUI();

  const Mouse &getMouse();
  PickingTexture &getPickingTexture();

private:
  GLFWwindow *_window;
  std::shared_ptr<Scene> _scene;
  std::vector<std::shared_ptr<IEntity>> _selectedEntities;
  std::vector<std::shared_ptr<IController>> _controllers;
  ControllerKind _selectedController = ControllerKind::Camera;
  ControllMode _controllMode = ControllMode::Transformation;
  CenterPoint _centerPoint;
  Mouse _mouse;
  GuiVisitor _guiSettingsVisitor;

  std::unordered_map<EntityType, std::shared_ptr<IEntityFactory>>
      _entityFactories;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initEnitityFactories();
  void initControllers();

  void renderModelControllSettings();
  void renderControllModeSettings();
  void renderCursorControllerSettings();

  void renderCreateTorusUI();
  void renderCreatePointUI();
  void removeButtonUI();
  void createBezierCurveUI();
  void createBezierCurveC2UI();

  IEntity &createEntity(EntityType entityType);
  void createBezierCurve();
  void createBezierCurveC2();
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
  std::vector<std::reference_wrapper<BezierCurveC0>> getSelectedBezierCurves();
};