#pragma once

#include "IController.hpp"
#include "cursor.hpp"
#include "mouse.hpp"

class CursorController : public IController {
public:
  CursorController(GLFWwindow *window, Camera *camera)
      : _window(window), _cursor(std::make_shared<Cursor>()), _camera(camera) {}

  bool processMouse() override { return false; }
  bool processScroll() override { return false; }

  void translate(float mouseX, float MouseY) {
    float x = (2.f * mouseX) / GLFWHelper::getWidth(_window) - 1.f;
    float y = 1.f - (2.f * MouseY) / GLFWHelper::getHeight(_window);

    _cursor.get()->updatePosition(x, y, *_camera);
  }

  void process(float x, float y) override {
    ImVec2 currentMousePosition = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
      translate(currentMousePosition.x, currentMousePosition.y);
  }

  Cursor &getCursor() { return *_cursor; }

private:
  std::shared_ptr<Cursor> _cursor;
  GLFWwindow *_window;
  Camera *_camera;
};