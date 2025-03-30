#pragma once
#include "GLFW/glfw3.h"
#include "controllers.hpp"
#include "imgui.h"
#include "pointModel.hpp"
#include "torusModel.hpp"
#include <chrono>
#include <unordered_set>

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2 };

class GUI {
public:
  GUI(GLFWwindow *window, Camera *camera) : _window(window), _camera(camera) {
    initControllers();
  }

  IController &getController() {
    return *_controllers[static_cast<int>(_selectedController)];
  }

  const std::vector<IEntity *> &getEntities() const { return _entities; }

  std::vector<IEntity *> getSelectedEntities() {
    std::vector<IEntity *> selectedEntities;
    for (auto selectedEntityIndex : _selectedEntities) {
      selectedEntities.push_back(_entities[selectedEntityIndex]);
    }
    return selectedEntities;
  }

  Cursor &getCursor() {
    auto cursorController = std::dynamic_pointer_cast<CursorController>(
        _controllers[static_cast<int>(ControllerKind::Cursor)]);
    return cursorController.get()->getCursor();
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
      renderControllerUI();
      displayEntitiesList();
      renderShowCenterPointUI();
      renderCreateTorusUI();
      renderCreatePointUI();
      removeButtonUI();

      ImGui::End();
    }
  }

private:
  GLFWwindow *_window;
  Camera *_camera;
  std::unordered_set<uint32_t> _selectedEntities;
  std::vector<IEntity *> _entities;
  std::shared_ptr<IController> _controllers[3];
  ControllerKind _selectedController = ControllerKind::Camera;
  bool _showCenterPoint = false;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initControllers() {
    _controllers[static_cast<int>(ControllerKind::Camera)] =
        std::make_shared<CameraController>(_window, _camera);

    if (!_selectedEntities.empty())
      initModelController();

    _controllers[static_cast<int>(ControllerKind::Cursor)] =
        std::make_shared<CursorController>(_window, _camera);
  }

  void renderControllerUI() {
    const char *controllerOptions[] = {"Camera", "Model", "Cursor"};
    int selectedIndex = static_cast<int>(_selectedController);

    if (ImGui::Combo("Controller", &selectedIndex, controllerOptions,
                     IM_ARRAYSIZE(controllerOptions))) {
      if (_controllers[static_cast<int>(ControllerKind::Model)] != nullptr ||
          selectedIndex != 1) {
        _selectedController = static_cast<ControllerKind>(selectedIndex);
      }
    }

    if (_selectedController == ControllerKind::Model) {
      if (_controllers[static_cast<int>(ControllerKind::Model)] != nullptr)
        renderModelControllSettings();

    } else if (_selectedController == ControllerKind::Cursor)
      renderCursorControllerSettings();
  }

  void renderModelControllSettings() {
    const char *AxisOptions[] = {"X axis", "Y axis", "Z axis"};

    auto modelController = std::dynamic_pointer_cast<ModelController>(
        _controllers[static_cast<int>(ControllerKind::Model)]);

    if (modelController) {
      int selectedIndex =
          static_cast<int>(modelController->_transformationAxis);
      if (ImGui::Combo("TransformationAxis", &selectedIndex, AxisOptions,
                       IM_ARRAYSIZE(AxisOptions))) {
        modelController->_transformationAxis = static_cast<Axis>(selectedIndex);
      }
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

  void renderShowCenterPointUI() {
    ImGui::Checkbox("Show Center Point", &_showCenterPoint);
  };

  void createTorus() {
    auto cursorPosition = getCursor().getPosition();
    auto torus = new TorusModel(2.f, 1.f, cursorPosition);
    addEntity(torus);
  }

  void createPoint() {
    auto cursorPosition = getCursor().getPosition();
    auto point = new PointModel(cursorPosition);
    addEntity(point);
  }

  void addEntity(IEntity *entity) {
    _entities.push_back(entity);
    selectEntity(_entities.size() - 1);
  }

  void renderModelSettings() {
    if (_selectedEntities.size() != 1)
      return;

    auto selectedEntity = _entities[*_selectedEntities.begin()];
    if (selectedEntity->renderSettings())
      selectedEntity->updateMesh();
  }

  void displayEntitiesList() {
    for (int i = 0; i < _entities.size(); ++i) {

      auto name = _entities[i]->getName();
      name = name.empty() ? "##" : name;
      bool isSelected = _selectedEntities.find(i) != _selectedEntities.end();

      if (ImGui::Selectable(name.c_str(), isSelected,
                            ImGuiSelectableFlags_AllowDoubleClick)) {
        if (ImGui::GetIO().KeyCtrl) {
          if (isSelected)
            _selectedEntities.erase(i);
          else
            _selectedEntities.insert(i);

        } else {
          _selectedEntities.clear();
          _selectedEntities.insert(i);
        }
      }
    }
  }

  void initModelController() {
    _controllers[static_cast<int>(ControllerKind::Model)] =
        std::make_shared<ModelController>(
            _entities[*_selectedEntities.begin()]);
  }

  void deleteSelectedEntities() {
    if (_selectedEntities.empty())
      return;

    std::vector<int> sortedIndices(_selectedEntities.begin(),
                                   _selectedEntities.end());
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());

    for (int entityIndex : sortedIndices) {
      if (entityIndex >= 0 && entityIndex < _entities.size()) {
        _entities.erase(_entities.begin() + entityIndex);
      }
    }

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

  void selectEntity(int entityIndex) {
    _selectedEntities.insert(entityIndex);
    auto entity = _entities[entityIndex];

    auto &controller = _controllers[static_cast<int>(ControllerKind::Model)];
    if (controller == nullptr)
      initModelController();

    auto modelController =
        std::dynamic_pointer_cast<ModelController>(controller);

    modelController->updateEntity(entity);
  }
};