#pragma once
#include "IController.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include "vec.hpp"

class CameraController : public IController {
public:
  CameraController(GLFWwindow *window, Camera *camera)
      : _window(window), _mouse(Mouse::getInstance()), _camera(camera) {}

  Camera *getCamera() const { return _camera; }

  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _camera->changeZoom(-scroll * zoomSpeed);
    return true;
  }

  bool processMouse() override { return processLeftButton(); }

private:
  Camera *_camera;
  GLFWwindow *_window;
  Mouse &_mouse;

  const float cameraSpeed = M_PI / 400.f;
  const float cameraMoveSpeed = 0.002f;
  const float zoomSpeed = 0.1f;

  bool processLeftButton() {
    if (!ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      ImVec2 currentMousePosition = ImGui::GetMousePos();

      if (!_mouse._leftClicked) {
        _mouse._leftClicked = true;
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
      }
      if (_mouse._leftClicked) {
        float deltaY = _mouse._position[1] - currentMousePosition.y;
        float deltaX = _mouse._position[0] - currentMousePosition.x;
        if (deltaY == 0.f && deltaX == 0.f)
          return false;

        if (ImGui::GetIO().KeyShift == false) {
          _camera->rotateHorizontal(deltaX * cameraSpeed);
          _camera->rotateVertical(deltaY * cameraSpeed);
        } else
          _camera->updateTarget(-deltaX * cameraMoveSpeed,
                                deltaY * cameraMoveSpeed);
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      _mouse._leftClicked = false;
    }
    return false;
  }
};