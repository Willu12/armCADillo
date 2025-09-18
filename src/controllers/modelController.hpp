#pragma once
#include "IController.hpp"
#include "IEntity.hpp"
#include "camera.hpp"
#include "centerPoint.hpp"
#include "cursor.hpp"
#include "imgui.h"
#include "mouse.hpp"
#include <memory>

enum class Axis : uint8_t { X = 0, Y = 1, Z = 2 };

enum class TransformationCenter : uint8_t { CenterPoint = 0, Cursor = 1 };

class ModelController : public IController {
public:
  Axis _transformationAxis = Axis::X;
  TransformationCenter _transformationCenter =
      TransformationCenter::CenterPoint;

  ModelController(const CenterPoint &centerPoint,
                  const std::shared_ptr<Cursor> &cursor,
                  const std::vector<std::shared_ptr<IEntity>> &entites,
                  const Camera &camera)
      : _entites(entites), _centerPoint(centerPoint), _cursor(cursor),
        _camera(camera) {}

  bool processScroll() override { return false; }
  bool processMouse() override { return false; }

  void process(const Mouse &mouse) override {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      if (ImGui::GetIO().KeyShift) {
        rotateAroundCenterPoint(mouse.getPositionDelta()[1]);
      } else if (ImGui::GetIO().KeyAlt) {
        scaleAroundCenterPoint(mouse.getPositionDelta()[1]);
      } else {
        ImVec2 currentMousePosition = ImGui::GetMousePos();
        translate(currentMousePosition.x, currentMousePosition.y);
      }
    }
  }

private:
  const std::vector<std::shared_ptr<IEntity>> &_entites;
  static constexpr float kMoveSpeed = 0.01f;
  const CenterPoint &_centerPoint;
  const std::shared_ptr<Cursor> _cursor;
  const Camera &_camera;

  void translate(float x, float y) {
    auto &window = _camera.getWindow();
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
      return;
    }
    x = (2.f * x) / static_cast<float>(GLFWHelper::getWidth(&window)) - 1.f;
    y = 1.f - (2.f * y) / static_cast<float>(GLFWHelper::getHeight(&window));
    if (_entites.size() == 1) {
      updateFromCamera(_entites.front(), x, y);
    } else {
      updateCenterPoint(x, y);
    }
  }

  void rotateAroundCenterPoint(float deltaY) {
    auto quaternion = algebra::Quaternion<float>::fromAxisAngle(
        getAxisVector(_transformationAxis), deltaY * kMoveSpeed);

    for (const auto &entity : _entites) {
      entity->rotateAroundPoint(quaternion, getTransformationPoint());
    }
  }

  void scaleAroundCenterPoint(float deltaY) {
    if (deltaY == 0.f) {
      return;
    }

    const auto scaleFactor = deltaY > 0.f ? 1.02f : 0.98f;
    for (const auto &entity : _entites) {
      if (entity->getScale()[0] * scaleFactor > 0.001f) {
        entity->scaleAroundPoint(scaleFactor, getTransformationPoint());
      }
    }
  }

  algebra::Vec3f getAxisVector(const Axis &axis) {
    if (axis == Axis::X) {
      return algebra::Vec3f(1.f, 0.f, 0.f);
    }
    if (axis == Axis::Y) {
      return algebra::Vec3f(0.f, 1.f, 0.f);
    }

    return algebra::Vec3f(0.f, 0.f, 1.f);
  }

  algebra::Vec3f getTransformationPoint() const {
    return _transformationCenter == TransformationCenter::CenterPoint
               ? _centerPoint.getPosition()
               : _cursor->getPosition();
  }

  void updateFromCamera(const std::shared_ptr<IEntity> &entity, float x,
                        float y) {
    auto projection = _camera.projectionMatrix();
    auto sceneCursorPosition =
        projection *
        (_camera.viewMatrix() * entity->getPosition().toHomogenous());

    sceneCursorPosition = sceneCursorPosition * (1.0f / sceneCursorPosition[3]);
    float z_ndc = sceneCursorPosition[2];

    auto screenPosition = algebra::Vec3f(x, y, z_ndc).toHomogenous();

    auto viewPosition = _camera.inverseProjectionMatrix() * screenPosition;
    viewPosition = viewPosition * (1.f / viewPosition[3]);
    auto worldPos = _camera.inverseViewMatrix() * viewPosition;
    entity->updatePosition(worldPos.fromHomogenous());
  }

  void updateCenterPoint(float x, float y) {
    auto projection = _camera.projectionMatrix();
    auto basePosition = _centerPoint.getPosition();
    auto sceneCursorPosition =
        projection *
        (_camera.viewMatrix() * _centerPoint.getPosition().toHomogenous());

    sceneCursorPosition = sceneCursorPosition * (1.0f / sceneCursorPosition[3]);
    float z_ndc = sceneCursorPosition[2];

    auto screenPosition = algebra::Vec3f(x, y, z_ndc).toHomogenous();

    auto viewPosition = _camera.inverseProjectionMatrix() * screenPosition;
    viewPosition = viewPosition * (1.f / viewPosition[3]);
    auto worldPos = _camera.inverseViewMatrix() * viewPosition;
    auto diff = worldPos.fromHomogenous() - basePosition;

    for (const auto &entity : _entites) {
      entity->updatePosition(entity->getPosition() + diff);
    }
  }
};