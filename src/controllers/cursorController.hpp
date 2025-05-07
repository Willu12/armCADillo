#pragma once

#include "IController.hpp"
#include "cursor.hpp"
#include "imgui.h"
#include <memory>
#include <utility>

class CursorController : public IController {
public:
  CursorController(GLFWwindow *window, std::shared_ptr<Camera> camera)
      : _window(window), _cursor(std::make_shared<Cursor>()),
        _camera(std::move(camera)) {}

  bool processMouse() override { return false; }
  bool processScroll() override { return false; }

  void translate(float mouseX, float MouseY) {
    float x =
        (2.f * mouseX) / static_cast<float>(GLFWHelper::getWidth(_window)) -
        1.f;
    float y = 1.f - (2.f * MouseY) /
                        static_cast<float>(GLFWHelper::getHeight(_window));

    _cursor->updatePosition(x, y, *_camera);
  }

  void process(float /*x*/, float /*y*/) override {
    ImVec2 currentMousePosition = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
      translate(currentMousePosition.x, currentMousePosition.y);
  }

  std::shared_ptr<Cursor> getCursor() { return _cursor; }

private:
  GLFWwindow *_window;
  std::shared_ptr<Cursor> _cursor;
  std::shared_ptr<Camera> _camera;
};