#pragma once
#include "IController.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "memory"

class CameraController : public IController {
public:
  CameraController(std::shared_ptr<Camera> camera) : _camera(camera) {}

  std::shared_ptr<Camera> getCamera() const { return _camera; }

  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _camera->changeZoom(-scroll * kZoomSpeed);
    return true;
  }

  bool processMouse() override { return false; }

  void rotate(float deltaX, float deltaY) {
    _camera->rotateHorizontal(deltaX * kCameraRotationSpeed);
    _camera->rotateVertical(deltaY * kCameraRotationSpeed);
  }

  void translate(float deltaX, float deltaY) {
    _camera->updateTarget(-deltaX * kCameraMoveSpeed,
                          deltaY * kCameraMoveSpeed);
  }

  void process(float x, float y) override {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      if (ImGui::GetIO().KeyShift)
        translate(x, y);
      else
        rotate(x, y);
    }
  }

private:
  std::shared_ptr<Camera> _camera;

  static constexpr float kCameraRotationSpeed = M_PI / 400.f;
  static constexpr float kCameraMoveSpeed = 0.002f;
  static constexpr float kZoomSpeed = 0.1f;
};