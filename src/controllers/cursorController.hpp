#pragma once

#include "IController.hpp"
#include "cursor.hpp"
#include "mouse.hpp"

class CursorController : public IController {
public:
  CursorController(GLFWwindow *window, Camera *camera)
      : _window(window), _cursor(std::make_unique<Cursor>()),
        _mouse(Mouse::getInstance()), _camera(camera) {}

  bool processMouse() override { return processLeftMouseButton(); }
  bool processScroll() override { return false; }

  const Cursor &getCursor() { return *_cursor; }

private:
  std::unique_ptr<Cursor> _cursor;
  Mouse &_mouse;
  GLFWwindow *_window;
  Camera *_camera;

  bool processLeftMouseButton() {
    if (!ImGui::IsAnyItemActive() &&
        (ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
         ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
      ImVec2 currentMousePosition = ImGui::GetMousePos();

      float x =
          (2.f * currentMousePosition.x) / GLFWHelper::getWidth(_window) - 1.f;
      float y =
          1.f - (2.f * currentMousePosition.y) / GLFWHelper::getHeight(_window);

      _cursor.get()->updatePosition(x, y, *_camera);
      _mouse._position =
          algebra::Vec2f(currentMousePosition.x, currentMousePosition.y);
      return true;
    }
    return false;
  }
};