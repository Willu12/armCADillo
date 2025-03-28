#pragma once
#include "GLFW/glfw3.h"
#include "controllers.hpp"
#include "imgui.h"
#include <chrono>

enum class ControllerKind { Camera = 0, Model = 1, Cursor = 2 };

class GUI {
public:
  //  GUI(GLFWwindow *window, Camera *camera) : _window(window), _camera(camera)
  //  {}

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

private:
  GLFWwindow *_window;
  Camera *_camera;
  IEntity *_entity;
  std::shared_ptr<IController> _controllers[3];

  std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime =
      std::chrono::high_resolution_clock::now();
  double _fps = 0.0;

  void initControllers() {
    _controllers[static_cast<int>(ControllerKind::Camera)] =
        std::make_shared<CameraController>(_window);

    /*_controllers[static_cast<int>(ControllerKind::Model)] =
        std::make_shared<ModelController>(_window);*/
  }
};