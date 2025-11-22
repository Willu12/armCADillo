#include "gui.hpp"
#include "IController.hpp"
#include "IDifferentialParametricForm.hpp"
#include "IEntity.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "bezierSurfaceRenderer.hpp"
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
#include "modelController.hpp"
#include "nfd.h"
#include "normalOffsetSurface.hpp"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "sceneRenderer.hpp"
#include "selectionController.hpp"
#include "utils.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

GUI::GUI(GLFWwindow *window, Scene *scene)
    : _window(window), _scene(scene), _entityFactory(_scene),
      _guiSettingsVisitor(*this), entityUtils_(this, &_entityFactory) {
  initControllers();
  pathsGenerator_.setIntersectionFinder(&_intersectionFinder);
  pathsGenerator_.setScene(scene);
}

IController &GUI::getController() {
  return *_controllers[static_cast<int>(_selectedController)];
}

std::vector<IEntity *> GUI::getEntities() const { return _scene->getEntites(); }

const std::vector<IEntity *> &GUI::getSelectedEntities() const {
  return _selectedEntities;
}
std::vector<IEntity *> GUI::getSelectedPointsPointers() const {
  std::vector<IEntity *> selected_points;
  for (const auto &entity : _selectedEntities) {
    if (dynamic_cast<PointEntity *>(entity)) {
      selected_points.emplace_back(entity);
    }
  }
  return selected_points;
}

Cursor *GUI::getCursor() {
  auto *cursor_controller = dynamic_cast<CursorController *>(
      _controllers[static_cast<int>(ControllerKind::Cursor)].get());

  return cursor_controller->getCursor();
}

const Cursor &GUI::getCursor() const {
  auto *cursor_controller = dynamic_cast<CursorController *>(
      _controllers[static_cast<int>(ControllerKind::Cursor)].get());
  return *cursor_controller->getCursor();
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
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
  ImGui::SetNextWindowBgAlpha(0.9f);
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Settings", nullptr, window_flags)) {
    showFPSCounter();

    renderModelSettings();
    renderModelControllSettings();
    selectAllPointsUI();
    displayEntitiesList();
    renderCursorControllerSettings();

    createEntityUI();
    findIntersectionUI();

    removeButtonUI();
    contractEdgeUI();

    ImGui::Separator();
    createSerializeUI();
    createLoadSceneUI();
    ImGui::Separator();
    stereoscopicSettings();
    processControllers();

    renderPathGeneratorUI();
    ImGui::End();
  }
  pathCombinerGUI_.displayGUI(getCursor());
}

const Mouse &GUI::getMouse() { return _mouse; }
PickingTexture &GUI::getPickingTexture() {
  return getSelectionController()->getPickingTexture();
}

void GUI::initControllers() {
  _controllers.resize(4);
  _controllers[static_cast<int>(ControllerKind::Camera)] =
      std::make_unique<CameraController>(_scene->getCamera());

  _controllers[static_cast<int>(ControllerKind::Cursor)] =
      std::make_unique<CursorController>(_window, _scene->getCamera());

  _controllers[static_cast<int>(ControllerKind::Model)] =
      std::make_unique<ModelController>(
          _centerPoint, getCursor(), _selectedEntities, *_scene->getCamera());
  _controllers[static_cast<int>(ControllerKind::Selection)] =
      std::make_unique<SelectionController>(_window, _scene, _selectedEntities);
}

void GUI::renderModelControllSettings() {
  const char *axis_options[] = {"X axis", "Y axis", "Z axis"};
  const char *transformation_center_options[] = {"Center Point", "Cursor"};

  auto *model_controller = dynamic_cast<ModelController *>(
      _controllers[static_cast<int>(ControllerKind::Model)].get());

  if (model_controller) {
    int selected_index =
        static_cast<int>(model_controller->_transformationAxis);
    if (ImGui::Combo("TransformationAxis", &selected_index, axis_options,
                     IM_ARRAYSIZE(axis_options))) {
      model_controller->_transformationAxis = static_cast<Axis>(selected_index);
    }

    selected_index = static_cast<int>(model_controller->_transformationCenter);
    if (ImGui::Combo("TransformationCenter", &selected_index,
                     transformation_center_options,
                     IM_ARRAYSIZE(transformation_center_options))) {
      model_controller->_transformationCenter =
          static_cast<TransformationCenter>(selected_index);
    }
  }
}

void GUI::renderCursorControllerSettings() {
  auto *cursor = getCursor();
  const auto &cursor_position = cursor->getPosition();
  float position[3] = {cursor_position[0], cursor_position[1],
                       cursor_position[2]};
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
    nfdu8char_t *out_path = nullptr;
    // nfdu8filteritem_t filters[1] = {{"serializedFile", "json"}};
    nfdopendialogu8args_t args = {nullptr};
    nfdresult_t result = NFD_OpenDialogU8_With(&out_path, &args);
    if (result == NFD_OKAY) {
      _jsonDeserializer.loadScence(std::string(out_path), *_scene);
      NFD_FreePathU8(out_path);
    }
    NFD_Quit();
  }
}

void GUI::createSerializeUI() {
  if (ImGui::Button("Save scene")) {
    NFD_Init();
    nfdu8char_t *out_path = nullptr;
    nfdsavedialogu8args_t args = {nullptr};
    nfdresult_t result = NFD_SaveDialogU8_With(&out_path, &args);
    if (result == NFD_OKAY) {
      _jsonSerializer.getSavePath() = std::string(out_path);
      NFD_FreePathU8(out_path);
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
  auto *selected_entity = *_selectedEntities.begin();
  if (selected_entity->acceptVisitor(_guiSettingsVisitor)) {
    selected_entity->updateMesh();
  }
}

void GUI::displayEntitiesList() {
  auto entities = _scene->getEntites();
  if (entities.empty()) {
    return;
  }
  const float height =
      entities.size() > 4 ? 200.f : 25.f * static_cast<float>(entities.size());
  ImVec2 child_size(-1, height);

  ImGui::BeginChild("EntitiesListChild", child_size, static_cast<int>(true),
                    ImGuiWindowFlags_HorizontalScrollbar);
  for (int i = 0; i < entities.size(); ++i) {

    auto name = entities[i]->getName();
    name = name.empty() ? "##" : name;
    bool is_selected = std::ranges::find(_selectedEntities, entities[i]) !=
                       _selectedEntities.end();

    if (ImGui::Selectable(name.c_str(), is_selected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (is_selected) {
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
  std::vector<const IEntity *> dead_entities;
  dead_entities.reserve(getSelectedEntities().size());
  for (const auto *entity : getSelectedEntities()) {
    if (const auto *point = dynamic_cast<const PointEntity *>(entity)) {
      if (point->surfacePoint()) {
        continue;
      }
    }
    dead_entities.push_back(entity);
  }

  clearSelectedEntities();

  _scene->removeEntities(dead_entities);
  // clearSelectedEntities();
}

void GUI::clearSelectedEntities() {
  // std::erase_if(_selectedEntities,
  //               [](IEntity *entity) { return entity == nullptr; });

  for (const auto &selected_entity : _selectedEntities) {
    selected_entity->setColor(Color::White());
  }
  _selectedEntities.clear();
}

void GUI::calculateFPS() {
  auto current_time = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> time_diff = current_time - _lastTime;
  if (time_diff.count() > 0.0) {
    _fps = 1.0 / time_diff.count();
    _lastTime = current_time;
  }
}

void GUI::showFPSCounter() {
  calculateFPS();
  ImGui::Text("FPS: %.1f", _fps);
}

ModelController *GUI::getModelController() {
  auto &controller = _controllers[static_cast<int>(ControllerKind::Model)];
  return dynamic_cast<ModelController *>(controller.get());
}

SelectionController *GUI::getSelectionController() {
  auto &controller = _controllers[static_cast<int>(ControllerKind::Selection)];
  return dynamic_cast<SelectionController *>(controller.get());
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

std::vector<IController *> GUI::getActiveControllers() {
  std::vector<IController *> active_controllers;

  active_controllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Camera)].get());
  active_controllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Cursor)].get());
  // active_controllers.push_back(
  //   _controllers[static_cast<int>(ControllerKind::Selection)]);

  active_controllers.push_back(
      _controllers[static_cast<int>(ControllerKind::Model)].get());

  return active_controllers;
}

std::vector<std::reference_wrapper<PointEntity>>
GUI::getSelectedPoints() const {
  std::vector<std::reference_wrapper<PointEntity>> point_entities;

  for (auto *entity : _selectedEntities) {
    if (auto *point = dynamic_cast<PointEntity *>(entity)) {
      point_entities.emplace_back(*point);
    }
  }
  return point_entities;
}

std::vector<std::reference_wrapper<PointEntity>> GUI::getPoints() const {
  std::vector<std::reference_wrapper<PointEntity>> point_entities;

  for (auto *entity : _scene->getPoints()) {
    if (auto *point = dynamic_cast<PointEntity *>(entity)) {
      point_entities.emplace_back(*point);
    }
  }
  return point_entities;
}
void GUI::processControllers() {
  auto selected_entities = getSelectedEntities();
  auto virtual_points = getSelectedVirtualPoints();
  _selectedEntities.insert(_selectedEntities.end(), virtual_points.begin(),
                           virtual_points.end());

  _selectedEntities = selected_entities;

  _controllers[static_cast<int>(ControllerKind::Selection)]->process(_mouse);
  _mouse.process(getActiveControllers());
}

void GUI::setVirtualPoints(
    const std::vector<VirtualPoint *> &virtualPoints,
    const std::vector<std::reference_wrapper<const VirtualPoint>>
        &selectedVirtualPoints) {
  _virtualPoints = virtualPoints;

  std::vector<VirtualPoint *> selected_virtual_points;

  for (const auto &ref : selectedVirtualPoints) {
    const VirtualPoint *virtual_point_ptr = &ref.get();

    auto it = std::ranges::find_if(_virtualPoints,
                                   [virtual_point_ptr](const VirtualPoint *vp) {
                                     return vp == virtual_point_ptr;
                                   });

    if (it != _virtualPoints.end()) {
      selected_virtual_points.push_back(*it);
    } else {
      throw std::runtime_error(
          "Selected VirtualPoint not found in _virtualPoints");
    }
  }
  _selectedVirtualPoints = selected_virtual_points;
}

std::vector<IEntity *> GUI::getSelectedVirtualPoints() const {
  std::vector<IEntity *> virtual_points;
  virtual_points.reserve(_selectedVirtualPoints.size());
  for (const auto &virtual_point : _selectedVirtualPoints) {
    virtual_points.emplace_back(virtual_point);
  }
  return virtual_points;
}

std::vector<IEntity *> GUI::getVirtualPoints() const {
  std::vector<IEntity *> virtual_points;
  virtual_points.reserve(_virtualPoints.size());
  for (const auto &virtual_point : _virtualPoints) {
    virtual_points.emplace_back(virtual_point);
  }
  return virtual_points;
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
  _scene->contractEdge(points[0], points[1]);
  _selectedEntities.clear();
  //_selectedEntities = {_scene->contractEdge(points[0], points[1])};
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
    if (auto *surface = dynamic_cast<BezierSurfaceC0 *>(entity)) {
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

  auto *surf0 =
      dynamic_cast<algebra::IDifferentialParametricForm<2, 3> *>(entities[0]);

  auto *surf1 =
      entities.size() == 1
          ? surf0
          : dynamic_cast<algebra::IDifferentialParametricForm<2, 3> *>(
                entities[1]);

  auto *bezier_surface_0 = dynamic_cast<BezierSurface *>(surf0);
  auto *bezier_surface_1 = dynamic_cast<BezierSurface *>(surf1);

  const auto offset_value =
      _intersectionFinder.getIntersectionConfig().offsetValue_;
  auto surf_0_offset = std::make_unique<algebra::NormalOffsetSurface>(
      &bezier_surface_0->getAlgebraSurfaceC0(), offset_value);
  auto surf_1_offset = std::make_unique<algebra::NormalOffsetSurface>(
      &bezier_surface_1->getAlgebraSurfaceC0(), offset_value);
  if (_intersectionFinder.getIntersectionConfig().useOffsetSurface_) {
    _intersectionFinder.setSurfaces(surf_0_offset.get(), surf_1_offset.get());
  } else {
    _intersectionFinder.setSurfaces(surf0, surf1);
  }
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

  auto intersection_curve = std::make_unique<IntersectionCurve>(
      *intersection, bounds, intersection->looped);

  intersection_curve->setFirstPoint(intersection->firstPoint);

  auto *surface_0_intersection = dynamic_cast<Intersectable *>(entities[0]);
  auto *surface_1_intersection =
      entities.size() == 1 ? surface_0_intersection
                           : dynamic_cast<Intersectable *>(entities[1]);

  intersection_curve->getFirstTexture().setWrapping(surf0->wrapped(0),
                                                    surf0->wrapped(1));
  intersection_curve->getSecondTexture().setWrapping(surf1->wrapped(0),
                                                     surf1->wrapped(1));

  surface_0_intersection->combineAndConnectIntersectionTexture(
      intersection_curve->getFirstTexturePtr());
  surface_1_intersection->combineAndConnectIntersectionTexture(
      intersection_curve->getSecondTexturePtr());

  _scene->addEntity(EntityType::IntersectionCurve,
                    std::move(intersection_curve));
}

void GUI::createEntityUI() {
  static EntityType selected_type = EntityType::Point;
  std::string preview_value = "Select...";

  for (const auto &[name, type] : entityUtils_.getStringEntityMap()) {
    if (type == selected_type) {
      preview_value = name;
      break;
    }
  }

  if (ImGui::BeginCombo("##Creatable Entities", preview_value.c_str())) {
    for (const auto &[name, entityType] : entityUtils_.getStringEntityMap()) {
      bool is_selected = (selected_type == entityType);
      if (ImGui::Selectable(name.c_str(), is_selected)) {
        selected_type = entityType;
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  entityUtils_.getEntityBuilders().at(selected_type)->drawGui();
}

void GUI::renderPathGeneratorUI() {
  ImGui::Begin("Path generation");
  if (ImGui::Button("set selected surfaces as model")) {
    pathsGenerator_.setModel(getSelectedSurfaces());
  }
  if (ImGui::Button("Generate Paths")) {
    pathsGenerator_.run();
  }

  static bool show_height_map_texture = false;

  auto render_texture_window = [](const std::string &window_name,
                                  uint32_t textureId) {
    ImGui::SetNextWindowSize(ImVec2(1500, 1500), ImGuiCond_Always);
    ImGui::Begin(window_name.c_str(), nullptr, ImGuiWindowFlags_NoResize);

    ImVec2 image_size(1500, 1500);
    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(textureId)),
                 image_size);
    ImGui::End();
  };

  ImGui::BeginDisabled(pathsGenerator_.heightMap() == nullptr);
  ImGui::Checkbox("Show heightMap Texture", &show_height_map_texture);
  if (show_height_map_texture) {
    render_texture_window("heightMap",
                          pathsGenerator_.heightMap()->textureId());
  }
  ImGui::EndDisabled();

  /// ------ it should be removed later ---------------------------------
  auto &bezier_surface_renderer =
      sceneRenderer_->getEntityRenderer(EntityType::BezierSurfaceC0);
  auto &bezier_surface_renderer_2 =
      sceneRenderer_->getEntityRenderer(EntityType::BezierSurfaceC2);

  auto *bezier_renderer_c0 =
      dynamic_cast<BezierSurfaceRenderer *>(&bezier_surface_renderer);
  auto *bezier_renderer_c2 =
      dynamic_cast<BezierSurfaceRenderer *>(&bezier_surface_renderer_2);

  if (ImGui::Checkbox("Show offset surface",
                      &bezier_renderer_c2->offsetSurface())) {
    bezier_renderer_c0->offsetSurface() = bezier_renderer_c2->offsetSurface();
    if (bezier_renderer_c2->offsetSurface()) {
      bezier_renderer_c0->normalDirection() = 1.f;
      bezier_renderer_c2->normalDirection() = -1.f;
    } else {

      bezier_renderer_c0->normalDirection() = 0.f;
      bezier_renderer_c2->normalDirection() = 0.f;
    }
  }

  ////
  auto &detailed_path_generator = pathsGenerator_.getDetailedPathGenerator();

  if (ImGui::Button("Prepare detailed path")) {
    detailed_path_generator.prepare();
  }

  if (ImGui::Button("generate detail path")) {
    detailed_path_generator.generate();
  }

  ImGui::End();
}

std::vector<BezierSurface *> GUI::getSelectedSurfaces() const {
  std::vector<BezierSurface *> surfaces;

  const auto &selected_entities = getSelectedEntities();
  for (auto *entity : selected_entities) {
    if (auto *surface = dynamic_cast<BezierSurface *>(entity)) {
      surfaces.push_back(surface);
    }
  }
  return surfaces;
}

void GUI::selectAllPointsUI() {
  if (ImGui::Button("Select all points")) {
    auto points = getPoints();
    for (const auto &point : points) {
      selectEntity(point);
    }
  }
}
