#include "gui.hpp"

#include "EntityFactories/pointFactory.hpp"
#include "EntityFactories/torusFactory.hpp"
#include "IEntity.hpp"
#include "bezierCurveC0.hpp"
#include "bezierCurveC2.hpp"
#include "imgui.h"
#include "scene.hpp"
#include "virtualPoint.hpp"
#include <cstdio>
#include <utility>
#include <vector>

GUI::GUI(GLFWwindow *window, std::shared_ptr<Scene> scene)
    : _window(window), _scene(std::move(scene)), _guiSettingsVisitor(*this) {
  initControllers();
  initEnitityFactories();
}

IController &GUI::getController() {
  return *_controllers[static_cast<int>(_selectedController)];
}

std::vector<std::shared_ptr<IEntity>> GUI::getEntities() const {
  return _scene->getEntites();
}

std::vector<std::shared_ptr<IEntity>> GUI::getSelectedEntities() const {
  return _selectedEntities;
}

std::shared_ptr<Cursor> GUI::getCursor() {
  auto cursorController = std::dynamic_pointer_cast<CursorController>(
      _controllers[static_cast<int>(ControllerKind::Cursor)]);
  return cursorController->getCursor();
}

std::optional<const IRenderable *> GUI::getCenterPoint() {
  if (_selectedEntities.size() > 0)
    _centerPoint.display(getSelectedEntities());

  if (_selectedEntities.size() < 2)
    return std::nullopt;

  return &_centerPoint.getPoint();
}

void GUI::displayGUI() {
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
  ImGui::SetNextWindowBgAlpha(0.9f);
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Settings", nullptr, window_flags)) {
    showFPSCounter();
    renderModelSettings();
    renderModelControllSettings();
    renderControllModeSettings();
    displayEntitiesList();

    renderCreateTorusUI();
    renderCreatePointUI();
    createBezierCurveUI();
    createBezierCurveC2UI();
    removeButtonUI();

    processControllers();

    ImGui::End();
  }
}

const Mouse &GUI::getMouse() { return _mouse; }
PickingTexture &GUI::getPickingTexture() {
  return getSelectionController()->getPickingTexture();
}

void GUI::initEnitityFactories() {
  _entityFactories.emplace(EntityType::Point, std::make_shared<PointFactory>());
  _entityFactories.emplace(EntityType::Torus, std::make_shared<TorusFactory>());
}

void GUI::initControllers() {
  _controllers.resize(4);
  _controllers[static_cast<int>(ControllerKind::Camera)] =
      std::make_shared<CameraController>(_scene->getCamera());

  _controllers[static_cast<int>(ControllerKind::Cursor)] =
      std::make_shared<CursorController>(_window, _scene->getCamera());

  _controllers[static_cast<int>(ControllerKind::Model)] =
      std::make_shared<ModelController>(_centerPoint, getCursor(),
                                        _selectedEntities);
  _controllers[static_cast<int>(ControllerKind::Selection)] =
      std::make_shared<SelectionController>(_window, _scene, _selectedEntities);
}

void GUI::renderModelControllSettings() {
  const char *AxisOptions[] = {"X axis", "Y axis", "Z axis"};
  const char *transformationCenterOptions[] = {"Center Point", "Cursor"};

  auto modelController = std::dynamic_pointer_cast<ModelController>(
      _controllers[static_cast<int>(ControllerKind::Model)]);

  if (modelController) {
    int selectedIndex = static_cast<int>(modelController->_transformationAxis);
    if (ImGui::Combo("TransformationAxis", &selectedIndex, AxisOptions,
                     IM_ARRAYSIZE(AxisOptions))) {
      modelController->_transformationAxis = static_cast<Axis>(selectedIndex);
    }

    selectedIndex = static_cast<int>(modelController->_transformationCenter);
    if (ImGui::Combo("TransformationCenter", &selectedIndex,
                     transformationCenterOptions,
                     IM_ARRAYSIZE(transformationCenterOptions))) {
      modelController->_transformationCenter =
          static_cast<TransformationCenter>(selectedIndex);
    }
  }
}

void GUI::renderControllModeSettings() {
  const char *controllModes[] = {"Transformation", "Selection"};
  int selectedIndex = static_cast<int>(_controllMode);

  if (ImGui::Combo("ControllMode", &selectedIndex, controllModes,
                   IM_ARRAYSIZE(controllModes))) {
    _controllMode = static_cast<ControllMode>(selectedIndex);
  }
}

void GUI::renderCursorControllerSettings() {
  auto cursor = getCursor();
  auto &cursorPosition = cursor->getPosition();
  float position[3] = {cursorPosition[0], cursorPosition[1], cursorPosition[2]};
  if (ImGui::InputFloat3("Cursor Position", position)) {
    cursor->updatePosition(position[0], position[1], position[2]);
  }
}

void GUI::renderCreateTorusUI() {
  if (ImGui::Button("Add new torus"))
    createEntity(EntityType::Torus);
}

void GUI::renderCreatePointUI() {

  if (ImGui::Button("Add new point")) {
    auto &entity = dynamic_cast<PointEntity &>(
        createEntity(EntityType::Point)); // std::static_cast()

    auto bezierCurves = getSelectedBezierCurves();
    for (const auto &bezierCurve : bezierCurves) {
      bezierCurve.get().addPoint(entity);
    }
  }
}

void GUI::removeButtonUI() {
  if (ImGui::Button("Remove Entity"))
    deleteSelectedEntities();
}

void GUI::createBezierCurveUI() {
  if (ImGui::Button("Create Bezier Curve"))
    createBezierCurve();
}
void GUI::createBezierCurveC2UI() {
  if (ImGui::Button("Create Bezier Curve C2"))
    createBezierCurveC2();
}

IEntity &GUI::createEntity(EntityType entityType) {
  const auto &cursorPosition = getCursor()->getPosition();
  auto entity = _entityFactories.at(entityType)->create(cursorPosition);
  _scene->addEntity(entityType, entity);
  return *entity;
}

void GUI::renderModelSettings() {
  if (_selectedEntities.size() != 1)
    return;

  auto selectedEntity = *_selectedEntities.begin();
  if (selectedEntity->acceptVisitor(_guiSettingsVisitor))
    selectedEntity->updateMesh();
}

void GUI::displayEntitiesList() {
  auto entities = _scene->getEntites();
  for (int i = 0; i < entities.size(); ++i) {

    auto name = entities[i]->getName();
    name = name.empty() ? "##" : name;
    bool isSelected = std::ranges::find(_selectedEntities, entities[i]) !=
                      _selectedEntities.end();

    if (ImGui::Selectable(name.c_str(), isSelected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected)
          unselectEntity(i);
        else
          selectEntity(i);

      } else {
        _selectedEntities.clear();
        selectEntity(i);
      }
    }
  }
}

void GUI::deleteSelectedEntities() {
  _scene->removeEntities(_selectedEntities);
  _selectedEntities.clear();
}

void GUI::calculateFPS() {
  auto currentTime = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> timeDiff = currentTime - _lastTime;
  if (timeDiff.count() > 0.0) {
    _fps = 1.0 / timeDiff.count();
    _lastTime = currentTime;
  }
}

void GUI::showFPSCounter() {
  calculateFPS();
  ImGui::Text("FPS: %.1f", _fps);
}

std::shared_ptr<ModelController> GUI::getModelController() {
  auto &controller = _controllers[static_cast<int>(ControllerKind::Model)];
  return std::dynamic_pointer_cast<ModelController>(controller);
}

std::shared_ptr<SelectionController> GUI::getSelectionController() {
  auto &controller = _controllers[static_cast<int>(ControllerKind::Selection)];
  return std::dynamic_pointer_cast<SelectionController>(controller);
}

void GUI::selectEntity(int entityIndex) {
  auto entities = _scene->getEntites();
  _selectedEntities.push_back(entities[entityIndex]);
}

void GUI::unselectEntity(int entityIndex) {
  _selectedEntities.erase(std::remove(_selectedEntities.begin(),
                                      _selectedEntities.end(),
                                      getEntities()[entityIndex]),
                          _selectedEntities.end());
}

std::vector<std::shared_ptr<IController>> GUI::getActiveControllers() {
  std::vector<std::shared_ptr<IController>> activeControllers;

  activeControllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Camera)]);
  activeControllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Cursor)]);

  if (_controllMode == ControllMode::Transformation)
    activeControllers.push_back(
        _controllers[static_cast<int>(ControllerKind::Model)]);

  return activeControllers;
}

std::vector<std::reference_wrapper<PointEntity>> GUI::getSelectedPoints() {
  std::vector<std::reference_wrapper<PointEntity>> pointEntities;

  for (auto &entity : _selectedEntities) {
    auto point = std::dynamic_pointer_cast<PointEntity>(entity);
    if (point) {
      pointEntities.emplace_back(*point);
    }
  }
  return pointEntities;
}

std::vector<std::reference_wrapper<PointEntity>> GUI::getPoints() const {
  std::vector<std::reference_wrapper<PointEntity>> pointEntities;

  for (auto &entity : _scene->getPoints()) {
    auto point = std::dynamic_pointer_cast<PointEntity>(entity);
    if (point) {
      pointEntities.emplace_back(*point);
    }
  }
  return pointEntities;
}

std::vector<std::reference_wrapper<BezierCurveC0>>
GUI::getSelectedBezierCurves() {
  std::vector<std::reference_wrapper<BezierCurveC0>> bezierCurves;
  for (auto &entity : _selectedEntities) {
    auto bezierCurve = std::dynamic_pointer_cast<BezierCurveC0>(entity);
    if (bezierCurve) {
      bezierCurves.emplace_back(*bezierCurve);
    }
  }
  return bezierCurves;
}

void GUI::createBezierCurve() {
  auto pointEntities = getSelectedPoints();
  if (pointEntities.size() < 2)
    return;

  std::shared_ptr<IEntity> bezierCurve =
      std::make_shared<BezierCurveC0>(pointEntities);
  _scene->addEntity(EntityType::BezierCurveC0, bezierCurve);
}

void GUI::createBezierCurveC2() {
  auto pointEntities = getSelectedPoints();
  if (pointEntities.size() < 2)
    return;

  std::shared_ptr<IEntity> bezierCurve =
      std::make_shared<BezierCurveC2>(pointEntities);
  _scene->addEntity(EntityType::BezierCurveC2, bezierCurve);
}

void GUI::processControllers() {

  auto selectedEntities = getSelectedEntities();
  auto vPoints = getSelectedVirtualPoints();
  _selectedEntities.insert(_selectedEntities.end(), vPoints.begin(),
                           vPoints.end());

  _mouse.process(getActiveControllers());
  if (_controllMode == ControllMode::Selection)
    _controllers[static_cast<int>(ControllerKind::Selection)]->process(0, 0);

  _selectedEntities = selectedEntities;
}

void GUI::setVirtualPoints(
    const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints,
    const std::vector<std::reference_wrapper<const VirtualPoint>>
        &selectedVirtualPoints) {
  _virtualPoints = virtualPoints;

  std::vector<std::shared_ptr<VirtualPoint>> selectedVirtualPointsPtrs;
  for (const auto &ref : selectedVirtualPoints) {
    const VirtualPoint *rawPtr = &ref.get();

    auto it = std::ranges::find_if(
        _virtualPoints, [rawPtr](const std::shared_ptr<VirtualPoint> &vp) {
          return vp.get() == rawPtr;
        });

    if (it != _virtualPoints.end()) {
      selectedVirtualPointsPtrs.push_back(*it);
    } else {
      throw std::runtime_error(
          "Selected VirtualPoint not found in _virtualPoints");
    }
  }
  _selectedVirtualPoints = selectedVirtualPointsPtrs;
}

std::vector<std::shared_ptr<IEntity>> GUI::getSelectedVirtualPoints() const {
  std::vector<std::shared_ptr<IEntity>> virtualPoints;
  virtualPoints.reserve(_selectedVirtualPoints.size());
  for (const auto &vPoint : _selectedVirtualPoints) {
    virtualPoints.emplace_back(vPoint);
  }
  return virtualPoints;
}

std::vector<std::shared_ptr<IEntity>> GUI::getVirtualPoints() const {
  std::vector<std::shared_ptr<IEntity>> virtualPoints;
  virtualPoints.reserve(_virtualPoints.size());
  for (const auto &vPoint : _virtualPoints) {
    virtualPoints.emplace_back(vPoint);
  }
  return virtualPoints;
}