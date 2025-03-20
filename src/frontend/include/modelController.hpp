#pragma once
#include "torusModel.hpp"

enum Axis { X = 0, Y = 1, Z = 2 };

class ModelController {
public:
  ModelController(TorusModel *torusModel) : _torusModel(torusModel) {}

  bool processScroll() {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    _torusModel->getScale() += scroll * _scrollSpeed;
    return true;
  }

private:
  TorusModel *_torusModel;
  Axis _transformationAxis = Axis::X;
  const float _scrollSpeed = 0.1f;
};