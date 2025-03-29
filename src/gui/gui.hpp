#pragma once
#include "GLFW/glfw3.h"
#include "controllers.hpp"
#include "imgui.h"
#include <chrono>

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2 };

class GUI {
public:
  GUI(GLFWwindow *window, Camera *camera, IEntity *_entity)
      : _window(window), _camera(camera) {
    _entities.push_back(_entity);
    initControllers();
    selectEntity(0);
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
    auto modelController = std::dynamic_pointer_cast<ModelController>(
        _controllers[static_cast<int>(ControllerKind::Model)]);

    if (modelController) {
      modelController->updateEntity(entity);
    }
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

      ImGui::End();
    }
  }

private:
  GLFWwindow *_window;
  Camera *_camera;
  int _selectedEntityIndex = 0;
  std::vector<IEntity *> _entities;
  std::shared_ptr<IController> _controllers[3];
  ControllerKind _selectedController = ControllerKind::Camera;

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initControllers() {
    _controllers[static_cast<int>(ControllerKind::Camera)] =
        std::make_shared<CameraController>(_window, _camera);

    _controllers[static_cast<int>(ControllerKind::Model)] =
        std::make_shared<ModelController>(_entities[_selectedEntityIndex]);

    _controllers[static_cast<int>(ControllerKind::Cursor)] =
        std::make_shared<CursorController>(_window, _camera);
  }

  void renderControllerUI() {
    const char *controllerOptions[] = {"Camera", "Model", "Cursor"};
    int selectedIndex = static_cast<int>(_selectedController);

    if (ImGui::Combo("Controller", &selectedIndex, controllerOptions,
                     IM_ARRAYSIZE(controllerOptions))) {
      _selectedController = static_cast<ControllerKind>(selectedIndex);
    }
    if (_selectedController == ControllerKind::Model) {
      renderModelControllSettings();
      auto cursor = getCursor();
      printf("cursor [%f]\n", cursor.getPosition()[0]);
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

  void renderModelSettings() {
    auto selectedEntity = _entities[_selectedEntityIndex];
    if (selectedEntity->renderSettings())
      selectedEntity->updateMesh();
  }

  void displayEntitiesList() {
    for (int i = 0; i < _entities.size(); ++i) {

      auto name = _entities[i]->getName();
      name = name.empty() ? "##" : name;
      if (ImGui::Selectable(name.c_str(), _selectedEntityIndex == i)) {
        _selectedEntityIndex = i;
      }
    }
  }
};