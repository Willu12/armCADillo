#pragma once
#include "IController.hpp"
#include "IEntity.hpp"

enum class Axis { X = 0, Y = 1, Z = 2 };

enum class TransformationCenter { CenterPoint = 0, Cursor = 1 };

class ModelController : public IController {
public:
  Axis _transformationAxis = Axis::X;
  TransformationCenter _transformationCenter =
      TransformationCenter::CenterPoint;

  ModelController(const CenterPoint &centerPoint, const Cursor &cursor,
                  const std::vector<IEntity *> &entites)
      : _centerPoint(centerPoint), _cursor(cursor), _entites(entites) {}
  bool processScroll() override {
    float scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0.0f)
      return false;
    for (auto &entity : _entites)
      entity->getScale() += scroll * _scrollSpeed;
    return true;
  }

  bool processMouse() override { return false; }

  void process(float x, float y) override {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      if (ImGui::GetIO().KeyShift)
        rotateAroundCenterPoint(y);
      else
        translate(y);
    }
  }

private:
  const std::vector<IEntity *> &_entites;
  const float _scrollSpeed = 0.01f;
  const float _moveSpeed = 0.01f;
  const CenterPoint &_centerPoint;
  const Cursor &_cursor;

  void translate(float deltaY) {
    for (auto &entity : _entites) {
      entity->getPosition()[static_cast<int>(_transformationAxis)] +=
          deltaY * _moveSpeed;
    }
  }

  void rotateAroundCenterPoint(float deltaY) {
    auto rotationPoint =
        _transformationCenter == TransformationCenter::CenterPoint
            ? _centerPoint.getPosition()
            : _cursor.getPosition();

    auto quaternion = algebra::Quaternion<float>::fromAxisAngle(
        getAxisVector(_transformationAxis), deltaY * _moveSpeed);
    for (auto &entity : _entites) {
      entity->rotateAroundPoint(quaternion, rotationPoint);
    }
  }

  /*
    void scaleAroundCenterPoint(float deltaY) {
      for (auto &entity : _entites) {
        entity->rotateAroundPoint(quaternion, _centerPoint.getPosition());
      }
    }
    */

  algebra::Vec3f getAxisVector(const Axis &axis) {
    if (axis == Axis::X)
      return algebra::Vec3f(1.f, 0.f, 0.f);
    else if (axis == Axis::Y)
      return algebra::Vec3f(0.f, 1.f, 0.f);
    else
      return algebra::Vec3f(0.f, 0.f, 1.f);
  }
};