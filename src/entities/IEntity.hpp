#pragma once
#include "IRenderable.hpp"
#include "IVisitor.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"

#include "matrix.hpp"
#include "quaternion.hpp"
#include "transformations.hpp"
#include "vec.hpp"

class GUI;

class IEntity : public IRenderable {
public:
  virtual ~IEntity() = default;

  virtual void updateMesh() = 0;
  virtual bool acceptVisitor(IVisitor &visitor) { return false; };

  virtual bool renderSettings(const GUI &gui) {
    ImGui::InputText("Name", &getName());

    float position[3] = {_position[0], _position[1], _position[2]};
    if (ImGui::InputFloat3("Position", position)) {
      updatePosition(algebra::Vec3f(position[0], position[1], position[2]));
      return true;
    }
    return false;
  }

  algebra::Vec3f &getScale() { return _scale; }
  const algebra::Vec3f &getScale() const { return _scale; }

  // algebra::Vec3f &getPosition() override { return _position; }
  void updatePosition(const algebra::Vec3f &position) override {
    _position = position;
  }
  const algebra::Vec3f &getPosition() const override { return _position; }

  algebra::Quaternion<float> &getRotation() { return _rotation; }
  const algebra::Quaternion<float> &getRotation() const { return _rotation; }

  std::string &getName() { return _name; }
  const std::string &getName() const { return _name; }

  algebra::Mat4f getModelMatrix() const override {
    auto scale = getScale();
    auto scaleMatrix = algebra::transformations::scaleMatrix(scale);
    auto rotationMatrix = getRotation().getRotationMatrix();
    auto translationMatrix =
        algebra::transformations::translationMatrix(getPosition());

    return translationMatrix * rotationMatrix * scaleMatrix;
  }

  virtual void rotateAroundPoint(const algebra::Quaternion<float> &rotation,
                                 const algebra::Vec3f &point) {
    auto rotatedAtOrigin = rotation * (_position - point);
    _position = rotatedAtOrigin.toVector() + point;
    _rotation = rotation * _rotation;
  }

  virtual void scaleAroundPoint(float scaleFactor,
                                const algebra::Vec3f &centerPoint) {
    auto translatedPosition = _position - centerPoint;
    translatedPosition = translatedPosition * scaleFactor;

    _scale = _scale * scaleFactor;
    _position = translatedPosition + centerPoint;
  }
  const uint32_t &getId() const { return _id; }
  uint32_t &getId() { return _id; }

protected:
  algebra::Vec3f _position;
  algebra::Quaternion<float> _rotation;
  algebra::Vec3f _scale = algebra::Vec3f(1.f, 1.f, 1.f);
  std::string _name;
  uint32_t _id;
};