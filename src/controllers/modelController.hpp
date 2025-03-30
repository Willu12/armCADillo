#pragma once
#include "IController.hpp"
#include "IEntity.hpp"

enum Axis { X = 0, Y = 1, Z = 2 };

class ModelController : public IController {
public:
  Axis _transformationAxis = Axis::X;

  ModelController(IEntity *entity)
      : _entity(entity), _mouse(Mouse::getInstance()) {}

  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _entity->getScale() += scroll * _scrollSpeed;
    return true;
  }

  bool processMouse() override {
    return processLeftButton() || processRightButton();
  }

  void updateEntity(IEntity *entity) { _entity = entity; }

private:
  IEntity *_entity;
  const float _scrollSpeed = 0.01f;
  const float _moveSpeed = 0.01f;
  Mouse &_mouse;

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

        rotateAroundLocalAxis(deltaY * _moveSpeed, _transformationAxis);
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

        _entity->getPosition()[_transformationAxis] += deltaY * _moveSpeed;
        _mouse._position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      _mouse._rightClicked = false;
    }
    return false;
  }

  algebra::Vec3f getAxisVector(const Axis &axis) {
    if (axis == Axis::X)
      return algebra::Vec3f(1.f, 0.f, 0.f);
    else if (axis == Axis::Y)
      return algebra::Vec3f(0.f, 1.f, 0.f);
    else
      return algebra::Vec3f(0.f, 0.f, 1.f);
  }

  void rotateAroundLocalAxis(float angle, const Axis &axis) {
    auto axisVector = getAxisVector(axis);
    auto quaternion =
        algebra::Quaternion<float>::fromAxisAngle(axisVector, angle);

    _entity->getRotation() = quaternion * _entity->getRotation();
  }
};