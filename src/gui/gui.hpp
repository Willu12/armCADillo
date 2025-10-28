#pragma once

#include "GuiVisitor.hpp"
#include "IController.hpp"
#include "IEntity.hpp"
#include "bezierSurfaceC0.hpp"
#include "centerPoint.hpp"
#include "controllers.hpp"
#include "cursor.hpp"
#include "entitiesTypes.hpp"
#include "entityFactory.hpp"
#include "intersectionFinder.hpp"
#include "jsonDeserializer.hpp"
#include "jsonSerializer.hpp"
#include "modelController.hpp"
#include "mouse.hpp"
#include "optional"
#include "pointEntity.hpp"
#include "selectionController.hpp"
#include "utils.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <chrono>
#include <functional>
#include <memory>
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
  GUI(GLFWwindow *window, Scene *scene);
  IController &getController();

  std::vector<std::shared_ptr<IEntity>> getEntities() const;
  std::vector<std::reference_wrapper<PointEntity>> getPoints() const;
  const std::vector<std::shared_ptr<IEntity>> &getSelectedEntities() const;
  std::vector<std::shared_ptr<IEntity>> getSelectedPointsPointers() const;
  std::vector<std::reference_wrapper<PointEntity>> getSelectedPoints() const;
  std::vector<std::reference_wrapper<BezierSurfaceC0>>
  getSelectedSurfacesC0() const;

  std::shared_ptr<Cursor> getCursor();
  const Cursor &getCursor() const;
  const algebra::Vec3f &getCursorPosition() const;

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
  bool &stereographicVision() { return _stereographicVision; }
  Scene &getScene() { return *_scene; }
  friend class GuiVisitor;

private:
  GLFWwindow *_window;
  Scene *_scene;
  EntityFactory _entityFactory;
  std::vector<std::shared_ptr<IEntity>> _selectedEntities;
  std::vector<std::shared_ptr<VirtualPoint>> _selectedVirtualPoints;
  std::vector<std::shared_ptr<VirtualPoint>> _virtualPoints;
  std::vector<std::unique_ptr<IController>> _controllers;
  ControllerKind _selectedController = ControllerKind::Camera;
  CenterPoint _centerPoint;
  Mouse _mouse;
  GuiVisitor _guiSettingsVisitor;
  JsonSerializer _jsonSerializer;
  JsonDeserializer _jsonDeserializer;
  IntersectionFinder _intersectionFinder;
  EntityUtils _entityUtils;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;
  bool _stereographicVision = false;

  void initControllers();
  void processControllers();

  void renderModelControllSettings();
  void renderCursorControllerSettings();

  void createEnitityUI();
  void removeButtonUI();
  void createSerializeUI();
  void createLoadSceneUI();
  void contractEdgeUI();
  void findIntersectionUI();
  void findIntersection();

  void renderModelSettings();
  void stereoscopicSettings();
  void displayEntitiesList();
  void deleteSelectedEntities();
  void clearSelectedEntities();
  void selectEntity(int entityIndex);
  void selectEntity(const IEntity &entity);
  void unselectEntity(int entityIndex);
  void contractSelectedEdge();

  void calculateFPS();
  void showFPSCounter();

  ModelController *getModelController();
  SelectionController *getSelectionController();
  std::vector<IController *> getActiveControllers();

  std::vector<std::reference_wrapper<BezierCurve>> getSelectedBezierCurves();
};
