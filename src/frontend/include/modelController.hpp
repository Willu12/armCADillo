#pragma once
#include "torusModel.hpp"

enum Axis { X = 0, Y = 1, Z = 2 };

class ModelController {
public:
  Axis _transformationAxis = Axis::X;

  ModelController(TorusModel *torusModel) : _torusModel(torusModel) {}

  bool processScroll() {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _torusModel->getScale() += scroll * _scrollSpeed;
    return true;
  }

  bool processMouse() { return processLeftButton() || processRightButton(); }

private:
  TorusModel *_torusModel;
  const float _scrollSpeed = 0.01f;
  const float _moveSpeed = 0.01f;
  Mouse _mouse;

  bool processLeftButton() {
    if (!ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
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

        _torusModel->getRotation().getRotation()[_transformationAxis] +=
            deltaY * _moveSpeed;
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      _mouse._leftClicked = false;
    }
    return false;
  }

  bool processRightButton() {
    if (!ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      ImVec2 currentMousePosition = ImGui::GetMousePos();

      if (!_mouse._rightClicked) {
        _mouse._rightClicked = true;
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
      }
      if (_mouse._rightClicked) {
        float deltaY = _mouse._position[1] - currentMousePosition.y;
        float deltaX = _mouse._position[0] - currentMousePosition.x;

        if (deltaY == 0.f && deltaX == 0.f)
          return false;

        _torusModel->getPosition()[_transformationAxis] += deltaY * _moveSpeed;
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      _mouse._rightClicked = false;
    }
    return false;
  }
};