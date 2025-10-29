#pragma once
#include <utility>

#include "IController.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "memory"
#include "mouse.hpp"

class CameraController : public IController {
public:
  explicit CameraController(Camera *camera) : _camera(camera) {}

  Camera *getCamera() const { return _camera; }

  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f) {
      return false;
    }
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

  void process(const Mouse &mouse) override {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      const auto &delta = mouse.getPositionDelta();
      if (ImGui::GetIO().KeyShift) {
        translate(delta[0], delta[1]);
      } else {
        rotate(delta[0], delta[1]);
      }
    }
  }

private:
  Camera *_camera = nullptr;

  static constexpr float kCameraRotationSpeed =
      std::numbers::pi_v<float> / 401.f;
  static constexpr float kCameraMoveSpeed = 0.05f;
  static constexpr float kZoomSpeed = 1.1f;
};