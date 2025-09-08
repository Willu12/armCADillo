#include "gui.hpp"
#include "IDifferentialParametricForm.hpp"
#include "IEntity.hpp"
#include "bezierSurfaceC0.hpp"
#include "color.hpp"
#include "cursor.hpp"
#include "cursorController.hpp"
#include "entitiesTypes.hpp"
#include "entityFactory.hpp"
#include "imgui.h"
#include "intersectable.hpp"
#include "intersectionCurve.hpp"
#include "intersectionFinder.hpp"
#include "jsonSerializer.hpp"
#include "nfd.h"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

GUI::GUI(GLFWwindow *window, std::shared_ptr<Scene> scene)
    : _window(window), _scene(std::move(scene)), _entityFactory(_scene.get()),
      _guiSettingsVisitor(*this), _entityUtils(this, &_entityFactory) {
  initControllers();
}

IController &GUI::getController() {
  return *_controllers[static_cast<int>(_selectedController)];
}

std::vector<std::shared_ptr<IEntity>> GUI::getEntities() const {
  return _scene->getEntites();
}

const std::vector<std::shared_ptr<IEntity>> &GUI::getSelectedEntities() const {
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

const Cursor &GUI::getCursor() const {
  auto cursorController = std::dynamic_pointer_cast<CursorController>(
      _controllers[static_cast<int>(ControllerKind::Cursor)]);
  return *cursorController->getCursor();
}

const algebra::Vec3f &GUI::getCursorPosition() const {
  return getCursor().getPosition();
}

std::optional<const IRenderable *> GUI::getCenterPoint() {
  if (_selectedEntities.size() > 0) {
    _centerPoint.display(getSelectedEntities());
  }

  if (_selectedEntities.size() < 2) {
    return std::nullopt;
  }

  return &_centerPoint.getPoint();
}

void GUI::displayGUI() {
  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
  ImGui::SetNextWindowBgAlpha(0.9f);
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Settings", nullptr, windowFlags)) {
    showFPSCounter();

    renderModelSettings();
    renderModelControllSettings();
    displayEntitiesList();
    renderCursorControllerSettings();

    createEnitityUI();
    findIntersectionUI();

    removeButtonUI();
    contractEdgeUI();

    ImGui::Separator();
    createSerializeUI();
    createLoadSceneUI();
    ImGui::Separator();
    stereoscopicSettings();
    processControllers();

    ImGui::End();
  }
}

const Mouse &GUI::getMouse() { return _mouse; }
PickingTexture &GUI::getPickingTexture() {
  return getSelectionController()->getPickingTexture();
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
  const char *axisOptions[] = {"X axis", "Y axis", "Z axis"};
  const char *transformationCenterOptions[] = {"Center Point", "Cursor"};

  auto modelController = std::dynamic_pointer_cast<ModelController>(
      _controllers[static_cast<int>(ControllerKind::Model)]);

  if (modelController) {
    int selectedIndex = static_cast<int>(modelController->_transformationAxis);
    if (ImGui::Combo("TransformationAxis", &selectedIndex, axisOptions,
                     IM_ARRAYSIZE(axisOptions))) {
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
  const auto &cursorPosition = cursor->getPosition();
  float position[3] = {cursorPosition[0], cursorPosition[1], cursorPosition[2]};
  if (ImGui::InputFloat3("Cursor Position", position)) {
    cursor->updatePosition(position[0], position[1], position[2]);
  }
}

void GUI::removeButtonUI() {
  if (ImGui::Button("Remove Entity")) {
    deleteSelectedEntities();
  }
}

void GUI::createLoadSceneUI() {
  if (ImGui::Button("Load Scene")) {
    NFD_Init();
    nfdu8char_t *outPath = nullptr;
    // nfdu8filteritem_t filters[1] = {{"serializedFile", "json"}};
    nfdopendialogu8args_t args = {nullptr};
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY) {
      _jsonDeserializer.loadScence(std::string(outPath), *_scene);
      NFD_FreePathU8(outPath);
    }
    NFD_Quit();
  }
}

void GUI::createSerializeUI() {
  if (ImGui::Button("Save scene")) {
    NFD_Init();
    nfdu8char_t *outPath = nullptr;
    nfdsavedialogu8args_t args = {nullptr};
    nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY) {
      _jsonSerializer.getSavePath() = std::string(outPath);
      NFD_FreePathU8(outPath);
      _jsonSerializer.serializeScene(*_scene);
    }
    NFD_Quit();
  }
}

void GUI::renderModelSettings() {
  if (_selectedEntities.size() != 1) {
    return;
  }

  clearVirtualPoints();
  auto selectedEntity = *_selectedEntities.begin();
  if (selectedEntity->acceptVisitor(_guiSettingsVisitor)) {
    selectedEntity->updateMesh();
  }
}

void GUI::displayEntitiesList() {
  auto entities = _scene->getEntites();
  if (entities.empty()) {
    return;
  }
  const float height =
      entities.size() > 4 ? 300.f : 25.f * static_cast<float>(entities.size());
  ImVec2 childSize(-1, height);

  ImGui::BeginChild("EntitiesListChild", childSize, static_cast<int>(true),
                    ImGuiWindowFlags_HorizontalScrollbar);
  for (int i = 0; i < entities.size(); ++i) {

    auto name = entities[i]->getName();
    name = name.empty() ? "##" : name;
    bool isSelected = std::ranges::find(_selectedEntities, entities[i]) !=
                      _selectedEntities.end();

    if (ImGui::Selectable(name.c_str(), isSelected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected) {
          unselectEntity(i);
        } else {
          selectEntity(i);
        }

      } else {
        clearSelectedEntities();
        selectEntity(i);
      }
    }
  }
  ImGui::EndChild();
}

void GUI::deleteSelectedEntities() {
  _scene->removeEntities(_selectedEntities);
  clearSelectedEntities();
}

void GUI::clearSelectedEntities() {
  for (const auto &selectedEntity : _selectedEntities) {
    selectedEntity->setColor(Color::White());
  }
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
  const auto &entities = _scene->getEntites();
  _selectedEntities.push_back(entities[entityIndex]);
  _selectedEntities.back()->setColor(Color::Orange());
}

void GUI::selectEntity(const IEntity &entity) {
  const auto &entities = _scene->getEntites();

  auto it = std::ranges::find_if(entities, [&entity](const auto &e) {
    return e->getId() == entity.getId() && e->getName() == entity.getName();
  });
  selectEntity(static_cast<int>(it - entities.begin()));
}

void GUI::unselectEntity(int entityIndex) {
  getEntities()[entityIndex]->setColor(Color::White());
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

std::vector<std::reference_wrapper<PointEntity>>
GUI::getSelectedPoints() const {
  std::vector<std::reference_wrapper<PointEntity>> pointEntities;

  for (const auto &entity : _selectedEntities) {
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

void GUI::stereoscopicSettings() {
  auto &camera = *_scene->getCamera();
  ImGui::Checkbox("Steroscopic Vision", &_stereographicVision);
  if (_stereographicVision) {
    ImGui::SliderFloat("eye distance", &camera.getEyeDistance(), 0.f, 1.f);
    ImGui::SliderFloat("convergence", &camera.getConvergence(), 0.1f, 10.f);
  }
}

void GUI::contractSelectedEdge() {
  const auto &points = getSelectedPoints();
  if (points.size() != 2) {
    return;
  }
  _selectedEntities = {_scene->contractEdge(points[0], points[1])};
}

void GUI::contractEdgeUI() {
  if (ImGui::Button("Contract Selected Edge")) {
    contractSelectedEdge();
  }
}

std::vector<std::reference_wrapper<BezierSurfaceC0>>
GUI::getSelectedSurfacesC0() const {
  std::vector<std::reference_wrapper<BezierSurfaceC0>> surfaces;

  for (const auto &entity : _selectedEntities) {
    if (auto surface = std::dynamic_pointer_cast<BezierSurfaceC0>(entity)) {
      surfaces.emplace_back(*surface);
    }
  }

  return surfaces;
}

void GUI::findIntersectionUI() {
  if (ImGui::Button("Find intersections")) {
    findIntersection();
  }
  _intersectionFinder.getIntersectionConfig().display();
}
void GUI::findIntersection() {
  auto entities = getSelectedEntities();
  if (entities.size() == 0) {
    return;
  }

  auto surf0 =
      std::dynamic_pointer_cast<algebra::IDifferentialParametricForm<2, 3>>(
          entities[0]);

  auto surf1 =
      entities.size() == 1
          ? surf0
          : std::dynamic_pointer_cast<
                algebra::IDifferentialParametricForm<2, 3>>(entities[1]);

  _intersectionFinder.setSurfaces(surf0, surf1);

  if (_intersectionFinder.getIntersectionConfig().useCursor_) {
    _intersectionFinder.setGuidancePoint(getCursorPosition());
  }

  auto intersection = _intersectionFinder.find(entities.size() == 1);
  if (!intersection) {
    return;
  }

  auto bounds1 = surf0->bounds();
  auto bounds2 = surf1->bounds();
  auto bounds =
      std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>(
          bounds1, bounds2);

  auto intersectionCurve = std::make_shared<IntersectionCurve>(
      *intersection, bounds, intersection->looped);
  intersectionCurve->setFirstPoint(intersection->firstPoint);

  _scene->addEntity(EntityType::IntersectionCurve, intersectionCurve);

  auto surfInter0 = std::dynamic_pointer_cast<Intersectable>(entities[0]);

  auto surfInter1 = entities.size() == 1
                        ? surfInter0
                        : std::dynamic_pointer_cast<Intersectable>(entities[1]);

  intersectionCurve->getFirstTexture().setWrapping(surf0->wrapped(0),
                                                   surf0->wrapped(1));
  intersectionCurve->getSecondTexture().setWrapping(surf1->wrapped(0),
                                                    surf1->wrapped(1));
  surfInter0->setIntersectionTexture(intersectionCurve->getFirstTexturePtr());
  surfInter1->setIntersectionTexture(intersectionCurve->getSecondTexturePtr());
}

void GUI::createEnitityUI() {
  static EntityType selectedType = EntityType::Point;
  std::string previewValue = "Select...";

  for (const auto &[name, type] : _entityUtils.getStringEntityMap()) {
    if (type == selectedType) {
      previewValue = name;
      break;
    }
  }

  if (ImGui::BeginCombo("##Creatable Entities", previewValue.c_str())) {
    for (const auto &[name, entityType] : _entityUtils.getStringEntityMap()) {
      bool isSelected = (selectedType == entityType);
      if (ImGui::Selectable(name.c_str(), isSelected)) {
        selectedType = entityType;
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  _entityUtils.getEntityBuilders().at(selectedType)->drawGui();
}