#pragma once
#include "IController.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include "vec.hpp"

class CameraController : IController {
public:
  CameraController() : _mouse(Mouse::getInstance()) { _camera = new Camera(); }

  Camera *getCamera() const { return _camera; }

  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _camera->changeZoom(-scroll * zoomSpeed);
    return true;
  }

  bool processMouse() override {
    return processLeftButton() || processRightButton();
  }

private:
  Camera *_camera;
  std::shared_ptr<Mouse> _mouse;

  const float cameraSpeed = M_PI / 400.f;
  const float cameraMoveSpeed = 0.002f;
  const float zoomSpeed = 0.1f;

  bool processLeftButton() {
    if (!ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      ImVec2 currentMousePosition = ImGui::GetMousePos();

      if (!_mouse.get()->_leftClicked) {
        _mouse.get()->_leftClicked = true;
        _mouse.get()->_position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
      }
      if (_mouse.get()->_leftClicked) {
        float deltaY = _mouse.get()->_position[1] - currentMousePosition.y;
        float deltaX = _mouse.get()->_position[0] - currentMousePosition.x;
        if (deltaY == 0.f && deltaX == 0.f)
          return false;

        _camera->rotateHorizontal(deltaX * cameraSpeed);
        _camera->rotateVertical(deltaY * cameraSpeed);
        _mouse.get()->_position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      _mouse.get()->_leftClicked = false;
    }
    return false;
  }

  bool processRightButton() {
    if (!ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      ImVec2 currentMousePosition = ImGui::GetMousePos();

      if (!_mouse.get()->_rightClicked) {
        _mouse.get()->_rightClicked = true;
        _mouse.get()->_position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
      }
      if (_mouse.get()->_rightClicked) {
        float deltaY = _mouse.get()->_position[1] - currentMousePosition.y;
        float deltaX = _mouse.get()->_position[0] - currentMousePosition.x;

        if (deltaY == 0.f && deltaX == 0.f)
          return false;

        _camera->updateTarget(-deltaX * cameraMoveSpeed,
                              deltaY * cameraMoveSpeed);
        _mouse.get()->_position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      _mouse.get()->_rightClicked = false;
    }
    return false;
  }
};