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
    selectEntity(_entity);
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

  void selectEntity(IEntity *entity) {
    _selectedEntity = entity;
    auto modelController = std::dynamic_pointer_cast<ModelController>(
        _controllers[static_cast<int>(ControllerKind::Model)]);

    if (modelController) {
      modelController->updateEntity(entity);
    }
  }

  const Cursor &getCursor() const {
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
      /*
      _change |=
          ImGui::SliderFloat("R", &_torusModel->getInnerRadius(), 0.1f, 10.f);
      _change |=
          ImGui::SliderFloat("r", &_torusModel->getTubeRadius(), 0.1f, 10.f);
      _change |= ImGui::SliderInt("Horizontal Density",
                                  &_torusModel->getMeshDensity().s, 3, 100);
      _change |= ImGui::SliderInt("Vertical Density",
                                  &_torusModel->getMeshDensity().t, 3, 100);
      */
      renderModelSettings();
      renderControllerUI();
      ImGui::End();
    }
  }

private:
  GLFWwindow *_window;
  Camera *_camera;
  IEntity *_selectedEntity;
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
        std::make_shared<ModelController>(_selectedEntity);

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
      const char *AxisOptions[] = {"X axis", "Y axis", "Z axis"};

      auto modelController = std::dynamic_pointer_cast<ModelController>(
          _controllers[static_cast<int>(ControllerKind::Model)]);

      if (modelController) {
        selectedIndex = static_cast<int>(modelController->_transformationAxis);
        if (ImGui::Combo("TransformationAxis", &selectedIndex, AxisOptions,
                         IM_ARRAYSIZE(AxisOptions))) {
          modelController->_transformationAxis =
              static_cast<Axis>(selectedIndex);
        }
      }
    }
  }

  void renderModelSettings() {
    if (_selectedEntity->renderSettings())
      _selectedEntity->updateMesh();
  }
};