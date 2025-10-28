#pragma once

#include "IController.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "vec.hpp"
#include <memory>

class Mouse {
public:
  void process(const std::vector<std::shared_ptr<IController>> &controllers) {
    if (ImGui::IsAnyItemHovered())
      return;
    processScroll(controllers);

    if (!ImGui::IsAnyItemActive() && anyButtonDown()) {
      if (!_clicked) {
        ImVec2 currentMousePosition = ImGui::GetMousePos();
        _clicked = true;
        _position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        _lastClickedPosition = _position;
      } else if (_clicked) {
        updateDelta();
        for (const auto &controller : controllers)
          if (controller)
            controller->process(*this);
      }
    } else if (AllButtonsUp()) {
      _clicked = false;
    }
  }

  bool anyButtonDown() const {
    return ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
           ImGui::IsMouseDown(ImGuiMouseButton_Middle) ||
           ImGui::IsMouseDown(ImGuiMouseButton_Right);
  }

  bool leftButtonDown() const {
    return ImGui::IsMouseDown(ImGuiMouseButton_Left);
  }

  const algebra::Vec2f &getPositionDelta() const { return _lastDelta; }
  const algebra::Vec2f &getCurrentPosition() const { return _position; }
  const algebra::Vec2f &getLastClickedPosition() const {
    return _lastClickedPosition;
  }

  // bool &isSelectionBoxActive() const { return _isSelectionBoxActive; }
  // const bool &isSelectionBoxActive() const { return _isSelectionBoxActive; }
  mutable bool _isSelectionBoxActive = false;

private:
  bool _clicked = false;
  algebra::Vec2f _position;
  algebra::Vec2f _lastClickedPosition;
  algebra::Vec2f _lastDelta;

  bool AllButtonsUp() const {
    return !ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
           !ImGui::IsMouseDown(ImGuiMouseButton_Middle) &&
           !ImGui::IsMouseDown(ImGuiMouseButton_Right);
  }

  void
  processScroll(const std::vector<std::shared_ptr<IController>> &controllers) {
    for (const auto &controller : controllers)
      if (controller)
        controller->processScroll();
  }

  void updateDelta() {
    ImVec2 currentMousePosition = ImGui::GetMousePos();
    float deltaY = _position[1] - currentMousePosition.y;
    float deltaX = _position[0] - currentMousePosition.x;
    if (deltaY == 0.f && deltaX == 0.f) {
      return;
    }

    _lastDelta = algebra::Vec2f{deltaX, deltaY};
    _position = algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
  }
};
