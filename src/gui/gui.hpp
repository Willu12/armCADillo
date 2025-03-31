#pragma once
#include "GLFW/glfw3.h"
#include "centerPoint.hpp"
#include "controllers.hpp"
#include "imgui.h"
#include "optional"
#include "pointEntity.hpp"
#include "polygonalCurve.hpp"
#include "selectionController.hpp"
#include "torusEntity.hpp"
#include <chrono>
#include <unordered_set>

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2, Selection = 3 };
enum class ControllMode { Transformation = 0, Selection = 1 };

class GUI {
public:
  GUI(GLFWwindow *window, Camera *camera) : _window(window), _camera(camera) {
    initControllers();
  }

  IController &getController() {
    return *_controllers[static_cast<int>(_selectedController)];
  }

  const std::vector<IEntity *> &getEntities() const { return _entities; }

  std::vector<IEntity *> getSelectedEntities() { return _entities; }

  Cursor &getCursor() {
    auto cursorController = std::dynamic_pointer_cast<CursorController>(
        _controllers[static_cast<int>(ControllerKind::Cursor)]);
    return cursorController.get()->getCursor();
  }

  std::optional<const IRenderable *> getCenterPoint() {

    if (_selectedEntities.size() == 0)
      return std::nullopt;

    _centerPoint.display(getSelectedEntities());
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
      createPolygonalCurveUI();
      removeButtonUI();

      _mouse.process(getActiveControllers());

      ImGui::End();
    }
  }

  const Mouse &getMouse() { return _mouse; }
  PickingTexture &getPickingTexture() {
    return getSelectionController()->getPickingTexture();
  }

  std::vector<IRenderable *> getPolygonalCurves() {
    std::vector<IRenderable *> renderables;
    for (auto polygonalCurve : _polygonalCurves) {
      renderables.push_back(polygonalCurve);
    }
    return renderables;
  }

private:
  GLFWwindow *_window;
  Camera *_camera;
  std::vector<IEntity *> _selectedEntities;
  std::vector<IEntity *> _entities;
  std::vector<PolygonalCurve *> _polygonalCurves;
  std::vector<std::shared_ptr<IController>> _controllers;
  ControllerKind _selectedController = ControllerKind::Camera;
  ControllMode _controllMode = ControllMode::Transformation;
  CenterPoint _centerPoint;
  Mouse _mouse;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initControllers() {
    _controllers.resize(4);
    _controllers[static_cast<int>(ControllerKind::Camera)] =
        std::make_shared<CameraController>(_window, _camera);

    _controllers[static_cast<int>(ControllerKind::Cursor)] =
        std::make_shared<CursorController>(_window, _camera);

    _controllers[static_cast<int>(ControllerKind::Model)] =
        std::make_shared<ModelController>(_centerPoint, getCursor(),
                                          _selectedEntities);
    _controllers[static_cast<int>(ControllerKind::Selection)] =
        std::make_shared<SelectionController>(_window, _entities,
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
    auto &cursor = getCursor();
    auto &cursorPosition = cursor.getPosition();
    float position[3] = {cursorPosition[0], cursorPosition[1],
                         cursorPosition[2]};
    if (ImGui::InputFloat3("Cursor Position", position)) {
      cursor.updatePosition(position[0], position[1], position[2]);
    }
  }

  void renderCreateTorusUI() {
    if (ImGui::Button("Add new torus"))
      createTorus();
  }

  void renderCreatePointUI() {
    if (ImGui::Button("Add new point"))
      createPoint();
  }

  void removeButtonUI() {
    if (ImGui::Button("Remove Entity"))
      deleteSelectedEntities();
  }

  void createPolygonalCurveUI() {
    if (ImGui::Button("Create Polygonal Curve"))
      createPolygonalCurve();
  }

  void createTorus() {
    auto cursorPosition = getCursor().getPosition();
    auto torus = new TorusEntity(2.f, 1.f, cursorPosition);
    addEntity(torus);
  }

  void createPoint() {
    auto cursorPosition = getCursor().getPosition();
    auto point = new PointEntity(cursorPosition);
    addEntity(point);
  }

  void addEntity(IEntity *entity) {
    _entities.push_back(entity);
    selectEntity(_entities.size() - 1);
  }

  void renderModelSettings() {
    if (_selectedEntities.size() != 1)
      return;

    auto selectedEntity = *_selectedEntities.begin();
    if (selectedEntity->renderSettings())
      selectedEntity->updateMesh();
  }

  void displayEntitiesList() {
    for (int i = 0; i < _entities.size(); ++i) {

      auto name = _entities[i]->getName();
      name = name.empty() ? "##" : name;
      bool isSelected =
          std::find(_selectedEntities.begin(), _selectedEntities.end(),
                    _entities[i]) != _selectedEntities.end();

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
    if (_selectedEntities.empty())
      return;

    _entities.erase(std::remove_if(_entities.begin(), _entities.end(),
                                   [&](IEntity *entity) {
                                     return std::find(_selectedEntities.begin(),
                                                      _selectedEntities.end(),
                                                      entity) !=
                                            _selectedEntities.end();
                                   }),
                    _entities.end());

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
    _selectedEntities.push_back(_entities[entityIndex]);
  }

  void unselectEntity(int entityIndex) {
    _selectedEntities.erase(std::remove(_selectedEntities.begin(),
                                        _selectedEntities.end(),
                                        _entities[entityIndex]),
                            _selectedEntities.end());
  }

  std::vector<std::shared_ptr<IController>> getActiveControllers() {
    std::vector<std::shared_ptr<IController>> activeControllers;

    activeControllers.push_back(
        _controllers[static_cast<int>(ControllerKind::Camera)]);
    activeControllers.push_back(
        _controllers[static_cast<int>(ControllerKind::Cursor)]);

    if (_controllMode == ControllMode::Selection)
      activeControllers.push_back(
          _controllers[static_cast<int>(ControllerKind::Selection)]);
    else
      activeControllers.push_back(
          _controllers[static_cast<int>(ControllerKind::Model)]);

    return activeControllers;
  }

  void createPolygonalCurve() {
    std::vector<PointEntity *> pointEntities;

    for (IEntity *entity : _selectedEntities) {
      PointEntity *point = dynamic_cast<PointEntity *>(entity);
      if (point) {
        pointEntities.push_back(point);
      }
    }
    if (pointEntities.size() < 2)
      return;
    _polygonalCurves.push_back(new PolygonalCurve(pointEntities));
  }
};