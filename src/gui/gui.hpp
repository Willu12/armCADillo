#pragma once
#include "GLFW/glfw3.h"
#include "controllers.hpp"
#include "imgui.h"
#include "point.hpp"
#include "torusModel.hpp"
#include <chrono>

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2 };

class GUI {
public:
  GUI(GLFWwindow *window, Camera *camera) : _window(window), _camera(camera) {
    initControllers();
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

  IController &getController() {
    return *_controllers[static_cast<int>(_selectedController)];
  }

  void selectEntity(int entityIndex) {
    _selectedEntityIndex = entityIndex;
    auto entity = _entities[entityIndex];

    auto &controller = _controllers[static_cast<int>(ControllerKind::Model)];
    if (controller == nullptr)
      initModelController();

    auto modelController =
        std::dynamic_pointer_cast<ModelController>(controller);

    modelController->updateEntity(entity);
  }

  const std::vector<IEntity *> &getEntities() const { return _entities; }

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
      renderCreateTorusUI();
      renderCreatePointUI();
      removeButtonUI();

      ImGui::End();
    }
  }

private:
  GLFWwindow *_window;
  Camera *_camera;
  std::optional<int> _selectedEntityIndex = std::nullopt;
  std::vector<IEntity *> _entities;
  std::shared_ptr<IController> _controllers[3];
  ControllerKind _selectedController = ControllerKind::Camera;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initControllers() {
    _controllers[static_cast<int>(ControllerKind::Camera)] =
        std::make_shared<CameraController>(_window, _camera);

    if (_selectedEntityIndex)
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
      deleteEntity();
  }

  void createTorus() {
    auto cursorPosition = getCursor().getPosition();
    auto torus = new TorusModel(2.f, 1.f, cursorPosition);
    addEntity(torus);
  }

  void createPoint() {
    auto cursorPosition = getCursor().getPosition();
    auto point = new Point(cursorPosition);
    addEntity(point);
  }

  void addEntity(IEntity *entity) {
    _entities.push_back(entity);
    selectEntity(_entities.size() - 1);
  }

  void renderModelSettings() {
    if (!_selectedEntityIndex)
      return;
    auto selectedEntity = _entities[_selectedEntityIndex.value()];
    if (selectedEntity->renderSettings())
      selectedEntity->updateMesh();
  }

  void displayEntitiesList() {
    for (int i = 0; i < _entities.size(); ++i) {

      auto name = _entities[i]->getName();
      name = name.empty() ? "##" : name;
      if (ImGui::Selectable(name.c_str(), _selectedEntityIndex.value() == i)) {
        selectEntity(i);
      }
    }
  }

  void initModelController() {
    _controllers[static_cast<int>(ControllerKind::Model)] =
        std::make_shared<ModelController>(
            _entities[_selectedEntityIndex.value()]);
  }

  void deleteEntity() {
    _entities.erase(_entities.begin() + _selectedEntityIndex.value());

    if (_entities.size() == 0) {
      _selectedEntityIndex = std::nullopt;
      return;
    }

    _selectedEntityIndex = _selectedEntityIndex.value() % _entities.size();
  }
};