#pragma once

#include "IController.hpp"
#include "cursor.hpp"
#include "mouse.hpp"

class CursorController : IController {
public:
  CursorController()
      : _cursor(std::make_unique<Cursor>()), _mouse(Mouse::getInstance()) {}

  bool processMouse() override { return processLeftMouseButton(); }
  bool processScroll() override { return false; }

  const Cursor &getCursor() { return *_cursor; }

private:
  std::unique_ptr<Cursor> _cursor;
  std::shared_ptr<Mouse> _mouse;

  bool processLeftMouseButton() {
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

        //   _camera->rotateHorizontal(deltaX * cameraSpeed);
        //   _camera->rotateVertical(deltaY * cameraSpeed);

        _cursor.get()->updatePosition(currentMousePosition.x,
                                      currentMousePosition.y);
        _mouse.get()->_position =
            algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
        return true;
      }
    } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      _mouse.get()->_leftClicked = false;
    }
    return false;
  }
};