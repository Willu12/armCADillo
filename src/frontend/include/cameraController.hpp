#pragma once
#include "camera.hpp"
#include "imgui.h"
#include "vec.hpp"

class Mouse {
public:
  algebra::Vec2f _position;
  bool _leftClicked = false;
};

class CameraController {
public:
  CameraController() { _camera = new Camera(); }

  Camera *getCamera() const { return _camera; }

  bool processMouse() {
    if (!ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
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

        _camera->rotateHorizontal(deltaX * cameraSpeed);
        _camera->rotateVertical(deltaY * cameraSpeed);
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (ImGui::IsItemHovered() &&
               !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      _mouse._leftClicked = false;
    }
    return false;
  }

private:
  Camera *_camera;
  Mouse _mouse;

  const float cameraSpeed = M_PI / 100.f;
};