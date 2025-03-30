#pragma once
#include "IController.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include "vec.hpp"

class CameraController : public IController {
public:
  CameraController(GLFWwindow *window, Camera *camera)
      : _window(window), _camera(camera) {}

  Camera *getCamera() const { return _camera; }

  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _camera->changeZoom(-scroll * zoomSpeed);
    return true;
  }

  bool processMouse() override { return false; }

  void rotate(float deltaX, float deltaY) {
    _camera->rotateHorizontal(deltaX * cameraSpeed);
    _camera->rotateVertical(deltaY * cameraSpeed);
  }

  void translate(float deltaX, float deltaY) {
    _camera->updateTarget(-deltaX * cameraMoveSpeed, deltaY * cameraMoveSpeed);
  }

  void process(float x, float y) {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      if (ImGui::GetIO().KeyShift)
        translate(x, y);
      else
        rotate(x, y);
    }
  }

private:
  Camera *_camera;
  GLFWwindow *_window;

  const float cameraSpeed = M_PI / 400.f;
  const float cameraMoveSpeed = 0.002f;
  const float zoomSpeed = 0.1f;
};