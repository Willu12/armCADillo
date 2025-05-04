#pragma once

#include "IController.hpp"
#include "imgui.h"
#include "vec.hpp"
#include <memory>

class Mouse {
public:
  void process(std::vector<std::shared_ptr<IController>> controllers) {
    processScroll(controllers);

    if (!ImGui::IsAnyItemActive() && anyButtonDown()) {
      ImVec2 currentMousePosition = ImGui::GetMousePos();

      if (!_clicked) {
        _clicked = true;
        _position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
      }
      if (_clicked) {
        float deltaY = _position[1] - currentMousePosition.y;
        float deltaX = _position[0] - currentMousePosition.x;
        if (deltaY == 0.f && deltaX == 0.f)
          return;

        for (const auto &controller : controllers)
          if (controller)
            controller->process(deltaX, deltaY);

        _position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
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

private:
  bool _clicked = false;
  algebra::Vec2f _position;

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
};
