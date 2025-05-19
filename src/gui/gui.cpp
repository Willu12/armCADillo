#include "gui.hpp"

#include "EntityFactories/pointFactory.hpp"
#include "EntityFactories/torusFactory.hpp"
#include "IEntity.hpp"
#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurfaceC0.hpp"
#include "bezierSurfaceC2.hpp"
#include "entitiesTypes.hpp"
#include "imgui.h"
#include "interpolatingSplineC2.hpp"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "virtualPoint.hpp"
#include <algorithm>
#include <cstdio>
#include <memory>
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
std::vector<std::shared_ptr<IEntity>> GUI::getSelectedPointsPointers() const {
  std::vector<std::shared_ptr<IEntity>> selectedPoints;
  for (const auto &entity : _selectedEntities) {
    if (std::dynamic_pointer_cast<PointEntity>(entity)) {
      selectedPoints.emplace_back(entity);
    }
  }
  return selectedPoints;
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
    displayEntitiesList();

    renderCreateTorusUI();
    renderCreatePointUI();
    createBezierCurveUI();
    createBSplineCurveUI();
    createInterpolatingSplineCurveUI();
    createBezierSurfaceC0UI();
    createBezierSurfaceC2UI();

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
      std::make_shared<ModelController>(
          _centerPoint, getCursor(), _selectedEntities, *_scene->getCamera());
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
void GUI::createBSplineCurveUI() {
  if (ImGui::Button("Create BSplineCurve"))
    createBSplineCurve();
}

void GUI::createInterpolatingSplineCurveUI() {
  if (ImGui::Button("Create Interpolating Spline Curve"))
    createInterpolatingSplineCurve();
}

void GUI::createBezierSurfaceC0UI() {
  static bool openConfigWindow = false;
  static int surfaceType = 0; // 0 = Flat, 1 = Cylinder

  // Default values
  static int u_patches = 2;
  static int v_patches = 2;

  static float radius = 1.0f;
  static float height = 2.0f;

  if (ImGui::Button("Create Bezier Surface C0")) {
    openConfigWindow = true;
  }

  if (openConfigWindow) {
    ImGui::Begin("Bezier Surface C0 Options", &openConfigWindow);

    ImGui::Text("Surface Type:");
    ImGui::RadioButton("Flat", &surfaceType, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Cylinder", &surfaceType, 1);

    ImGui::Separator();

    if (surfaceType == 0) {
      ImGui::InputInt("U Patches", &u_patches);
      ImGui::InputInt("V Patches", &v_patches);
    } else {
      ImGui::InputFloat("Radius", &radius);
      ImGui::InputFloat("Height", &height);
    }

    ImGui::Separator();

    if (ImGui::Button("Create")) {
      if (surfaceType == 0) {
        createBezierSurfaceC0Flat(u_patches, v_patches);
      } else {
        createBezierSurfaceC0Cylinder(radius, height);
      }

      openConfigWindow = false; // close after creation
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
      openConfigWindow = false;
    }

    ImGui::End();
  }
}

void GUI::createBezierSurfaceC2UI() {
  static bool openConfigWindow = false;
  static int surfaceType = 0; // 0 = Flat, 1 = Cylinder

  // Default values
  static int u_patches = 2;
  static int v_patches = 2;

  static float radius = 1.0f;
  static float height = 2.0f;

  if (ImGui::Button("Create Bezier Surface C2")) {
    openConfigWindow = true;
  }

  if (openConfigWindow) {
    ImGui::Begin("Bezier Surface C2 Options", &openConfigWindow);

    ImGui::Text("Surface Type:");
    ImGui::RadioButton("Flat", &surfaceType, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Cylinder", &surfaceType, 1);

    ImGui::Separator();

    if (surfaceType == 0) {
      ImGui::InputInt("U Patches", &u_patches);
      ImGui::InputInt("V Patches", &v_patches);
    } else {
      ImGui::InputFloat("Radius", &radius);
      ImGui::InputFloat("Height", &height);
    }

    ImGui::Separator();

    if (ImGui::Button("Create")) {
      if (surfaceType == 0) {
        createBezierSurfaceC2Flat(u_patches, v_patches);
      } else {
        // createBezierSurfaceC0Cylinder(radius, height);
      }

      openConfigWindow = false; // close after creation
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
      openConfigWindow = false;
    }

    ImGui::End();
  }
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

  clearVirtualPoints();
  auto selectedEntity = *_selectedEntities.begin();
  if (selectedEntity->acceptVisitor(_guiSettingsVisitor))
    selectedEntity->updateMesh();
}

void GUI::displayEntitiesList() {
  auto entities = _scene->getEntites();
  if (entities.empty())
    return;
  const float height = entities.size() > 4 ? 100.f : 25.f * entities.size();
  ImVec2 childSize(-1, height);

  ImGui::BeginChild("EntitiesListChild", childSize, true,
                    ImGuiWindowFlags_HorizontalScrollbar);
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
  ImGui::EndChild();
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
  _selectedEntities.erase(
      std::ranges::remove(_selectedEntities, getEntities()[entityIndex])
          .begin(),
      _selectedEntities.end());
}

std::vector<std::shared_ptr<IController>> GUI::getActiveControllers() {
  std::vector<std::shared_ptr<IController>> activeControllers;

  activeControllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Camera)]);
  activeControllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Cursor)]);
  // activeControllers.push_back(
  //   _controllers[static_cast<int>(ControllerKind::Selection)]);

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

std::vector<std::reference_wrapper<BezierCurve>>
GUI::getSelectedBezierCurves() {
  std::vector<std::reference_wrapper<BezierCurve>> bezierCurves;
  for (auto &entity : _selectedEntities) {
    auto bezierCurve = std::dynamic_pointer_cast<BezierCurve>(entity);
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

void GUI::createBSplineCurve() {
  auto pointEntities = getSelectedPoints();
  if (pointEntities.size() < 2)
    return;

  std::shared_ptr<IEntity> bezierCurve =
      std::make_shared<BSplineCurve>(pointEntities);
  _scene->addEntity(EntityType::BSplineCurve, bezierCurve);
}

void GUI::createInterpolatingSplineCurve() {
  auto pointEntities = getSelectedPoints();
  if (pointEntities.size() < 2)
    return;
  std::shared_ptr<IEntity> interpolatingSpline =
      std::make_shared<InterpolatingSplineC2>(pointEntities);
  _scene->addEntity(EntityType::InterpolatingSplineCurve, interpolatingSpline);
}

void GUI::createBezierSurfaceC0Flat(uint32_t uPatches, uint32_t vPatches) {
  const auto &cursorPosition = getCursor()->getPosition();

  std::shared_ptr<BezierSurfaceC0> bezierSurfaceC0 =
      BezierSurfaceC0::createFlat(cursorPosition, uPatches, vPatches);
  _scene->addEntity(EntityType::BezierSurfaceC0, bezierSurfaceC0);
  for (const auto &point : bezierSurfaceC0->getPoints()) {
    _scene->addEntity(EntityType::Point, point);
  }
}

void GUI::createBezierSurfaceC2Flat(uint32_t uPatches, uint32_t vPatches) {
  const auto &cursorPosition = getCursor()->getPosition();

  std::shared_ptr<BezierSurfaceC2> bezierSurfaceC2 =
      BezierSurfaceC2::createFlat(cursorPosition, uPatches, vPatches);
  _scene->addEntity(EntityType::BezierSurfaceC2, bezierSurfaceC2);
  for (const auto &point : bezierSurfaceC2->getPoints()) {
    _scene->addEntity(EntityType::Point, point);
  }
}

void GUI::createBezierSurfaceC0Cylinder(float r, float h) {
  const auto &cursorPosition = getCursor()->getPosition();

  std::shared_ptr<BezierSurfaceC0> bezierSurfaceC0 =
      BezierSurfaceC0::createCylinder(cursorPosition, r, h);
  _scene->addEntity(EntityType::BezierSurfaceC0, bezierSurfaceC0);
  for (const auto &point : bezierSurfaceC0->getPoints()) {
    _scene->addEntity(EntityType::Point, point);
  }
}

void GUI::processControllers() {

  auto selectedEntities = getSelectedEntities();
  auto vPoints = getSelectedVirtualPoints();
  _selectedEntities.insert(_selectedEntities.end(), vPoints.begin(),
                           vPoints.end());

  _selectedEntities = selectedEntities;

  _controllers[static_cast<int>(ControllerKind::Selection)]->process(_mouse);
  _mouse.process(getActiveControllers());
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

void GUI::clearVirtualPoints() {
  _selectedVirtualPoints.clear();
  _virtualPoints.clear();
}