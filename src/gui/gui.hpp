#pragma once

#include "EntityFactories/IEntityFactory.hpp"
#include "EntityFactories/pointFactory.hpp"
#include "EntityFactories/torusFactory.hpp"
#include "IEntity.hpp"
#include "bezierCurve.hpp"
#include "centerPoint.hpp"
#include "controllers.hpp"
#include "cursor.hpp"
#include "entitiesTypes.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include "optional"
#include "pointEntity.hpp"
#include "scene.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2, Selection = 3 };
enum class ControllMode { Transformation = 0, Selection = 1 };

class GUI {
public:
  GUI(GLFWwindow *window, std::shared_ptr<Scene> scene)
      : _window(window), _scene(scene) {
    initControllers();
    initEnitityFactories();
  }

  IController &getController() {
    return *_controllers[static_cast<int>(_selectedController)];
  }

  const std::vector<std::shared_ptr<IEntity>> getEntities() const {
    return _scene->getEntites();
  }

  std::vector<std::shared_ptr<IEntity>> getSelectedEntities() {
    return _selectedEntities;
  }

  std::shared_ptr<Cursor> getCursor() {
    auto cursorController = std::dynamic_pointer_cast<CursorController>(
        _controllers[static_cast<int>(ControllerKind::Cursor)]);
    return cursorController.get()->getCursor();
  }

  std::optional<const IRenderable *> getCenterPoint() {
    if (_selectedEntities.size() > 0)
      _centerPoint.display(getSelectedEntities());

    if (_selectedEntities.size() < 2)
      return std::nullopt;

    return &_centerPoint.getPoint();
  }

  void displayGUI() {
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
      removeButtonUI();

      _mouse.process(getActiveControllers());
      if (_controllMode == ControllMode::Selection)
        _controllers[static_cast<int>(ControllerKind::Selection)]->process(0,
                                                                           0);

      ImGui::End();
    }
  }

  const Mouse &getMouse() { return _mouse; }
  PickingTexture &getPickingTexture() {
    return getSelectionController()->getPickingTexture();
  }

private:
  GLFWwindow *_window;
  std::shared_ptr<Scene> _scene;
  std::vector<std::shared_ptr<IEntity>> _selectedEntities;
  std::vector<std::shared_ptr<IController>> _controllers;
  ControllerKind _selectedController = ControllerKind::Camera;
  ControllMode _controllMode = ControllMode::Transformation;
  CenterPoint _centerPoint;
  Mouse _mouse;

  std::unordered_map<EntityType, std::shared_ptr<IEntityFactory>>
      _entityFactories;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initEnitityFactories() {
    _entityFactories.emplace(EntityType::Point,
                             std::make_shared<PointFactory>());
    _entityFactories.emplace(EntityType::Torus,
                             std::make_shared<TorusFactory>());
  }

  void initControllers() {
    _controllers.resize(4);
    _controllers[static_cast<int>(ControllerKind::Camera)] =
        std::make_shared<CameraController>(_scene->getCamera());

    _controllers[static_cast<int>(ControllerKind::Cursor)] =
        std::make_shared<CursorController>(_window, _scene->getCamera());

    _controllers[static_cast<int>(ControllerKind::Model)] =
        std::make_shared<ModelController>(_centerPoint, getCursor(),
                                          _selectedEntities);
    _controllers[static_cast<int>(ControllerKind::Selection)] =
        std::make_shared<SelectionController>(_window, _scene,
                                              _selectedEntities);
  }

  void renderModelControllSettings() {
    const char *AxisOptions[] = {"X axis", "Y axis", "Z axis"};
    const char *transformationCenterOptions[] = {"Center Point", "Cursor"};

    auto modelController = std::dynamic_pointer_cast<ModelController>(
        _controllers[static_cast<int>(ControllerKind::Model)]);

    if (modelController) {
      int selectedIndex =
          static_cast<int>(modelController->_transformationAxis);
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

  void renderControllModeSettings() {
    const char *controllModes[] = {"Transformation", "Selection"};
    int selectedIndex = static_cast<int>(_controllMode);

    if (ImGui::Combo("ControllMode", &selectedIndex, controllModes,
                     IM_ARRAYSIZE(controllModes))) {
      _controllMode = static_cast<ControllMode>(selectedIndex);
    }
  }

  void renderCursorControllerSettings() {
    auto cursor = getCursor();
    auto &cursorPosition = cursor->getPosition();
    float position[3] = {cursorPosition[0], cursorPosition[1],
                         cursorPosition[2]};
    if (ImGui::InputFloat3("Cursor Position", position)) {
      cursor->updatePosition(position[0], position[1], position[2]);
    }
  }

  void renderCreateTorusUI() {
    if (ImGui::Button("Add new torus"))
      createEntity(EntityType::Torus);
  }

  void renderCreatePointUI() {

    if (ImGui::Button("Add new point")) {
      auto &entity = static_cast<PointEntity &>(
          createEntity(EntityType::Point)); // std::static_cast()

      auto bezierCurves = getSelectedBezierCurves();
      for (const auto &bezierCurve : bezierCurves) {
        bezierCurve.get().addPoint(entity);
      }
    }
  }

  void removeButtonUI() {
    if (ImGui::Button("Remove Entity"))
      deleteSelectedEntities();
  }

  void createBezierCurveUI() {
    if (ImGui::Button("Create Bezier Curve"))
      createBezierCurve();
  }

  IEntity &createEntity(EntityType entityType) {
    const auto &cursorPosition = getCursor()->getPosition();
    auto entity = _entityFactories.at(entityType)->create(cursorPosition);
    _scene->addEntity(entityType, entity);
    return *entity;
  }

  void renderModelSettings() {
    if (_selectedEntities.size() != 1)
      return;

    auto selectedEntity = *_selectedEntities.begin();
    if (selectedEntity->renderSettings())
      selectedEntity->updateMesh();
  }

  void displayEntitiesList() {
    auto entities = _scene->getEntites();
    for (int i = 0; i < entities.size(); ++i) {

      auto name = entities[i]->getName();
      name = name.empty() ? "##" : name;
      bool isSelected =
          std::find(_selectedEntities.begin(), _selectedEntities.end(),
                    entities[i]) != _selectedEntities.end();

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

  void deleteSelectedEntities() {
    // if (_selectedEntities.empty())
    //  return;
    _scene->removeEntities(_selectedEntities);
    _selectedEntities.clear();
  }

  void calculateFPS() {
    auto currentTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> timeDiff = currentTime - _lastTime;
    if (timeDiff.count() > 0.0) {
      _fps = 1.0 / timeDiff.count();
      _lastTime = currentTime;
    }
  }

  void showFPSCounter() {
    calculateFPS();
    ImGui::Text("FPS: %.1f", _fps);
  }

  std::shared_ptr<ModelController> getModelController() {
    auto &controller = _controllers[static_cast<int>(ControllerKind::Model)];
    return std::dynamic_pointer_cast<ModelController>(controller);
  }

  std::shared_ptr<SelectionController> getSelectionController() {
    auto &controller =
        _controllers[static_cast<int>(ControllerKind::Selection)];
    return std::dynamic_pointer_cast<SelectionController>(controller);
  }

  void selectEntity(int entityIndex) {
    auto entities = _scene->getEntites();
    _selectedEntities.push_back(entities[entityIndex]);
  }

  void unselectEntity(int entityIndex) {
    _selectedEntities.erase(std::remove(_selectedEntities.begin(),
                                        _selectedEntities.end(),
                                        getEntities()[entityIndex]),
                            _selectedEntities.end());
  }

  std::vector<std::shared_ptr<IController>> getActiveControllers() {
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

  std::vector<std::reference_wrapper<PointEntity>> getSelectedPoints() {
    std::vector<std::reference_wrapper<PointEntity>> pointEntities;

    for (auto entity : _selectedEntities) {
      auto point = std::dynamic_pointer_cast<PointEntity>(entity);
      if (point) {
        pointEntities.push_back(*point);
      }
    }
    return pointEntities;
  }

  std::vector<std::reference_wrapper<BezierCurve>> getSelectedBezierCurves() {
    std::vector<std::reference_wrapper<BezierCurve>> bezierCurves;
    for (auto entity : _selectedEntities) {
      auto bezierCurve = std::dynamic_pointer_cast<BezierCurve>(entity);
      if (bezierCurve) {
        bezierCurves.push_back(*bezierCurve);
      }
    }
    return bezierCurves;
  }

  void createBezierCurve() {
    auto pointEntities = getSelectedPoints();
    if (pointEntities.size() < 2)
      return;

    std::shared_ptr<IEntity> bezierCurve =
        std::make_shared<BezierCurve>(pointEntities);
    _scene->addEntity(EntityType::BezierCurve, bezierCurve);
  }
};