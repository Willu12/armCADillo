#include "IEntity.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "transformations.hpp"

bool IEntity::acceptVisitor(IVisitor &visitor) { return false; };

bool IEntity::renderSettings(const GUI &gui) {
  ImGui::InputText("Name", &getName());
  float position[3] = {_position[0], _position[1], _position[2]};

  if (ImGui::InputFloat3("Position", position)) {
    updatePosition(algebra::Vec3f(position[0], position[1], position[2]));
    return true;
  }

  return false;
}

algebra::Vec3f &IEntity::getScale() { return _scale; }
const algebra::Vec3f &IEntity::getScale() const { return _scale; }

void IEntity::updatePosition(const algebra::Vec3f &position) {
  _position = position;
}
const algebra::Vec3f &IEntity::getPosition() const { return _position; }

algebra::Quaternion<float> &IEntity::getRotation() { return _rotation; }
const algebra::Quaternion<float> &IEntity::getRotation() const {
  return _rotation;
}

std::string &IEntity::getName() { return _name; }
const std::string &IEntity::getName() const { return _name; }

algebra::Mat4f IEntity::getModelMatrix() const {
  auto scale = getScale();
  auto scaleMatrix = algebra::transformations::scaleMatrix(scale);
  auto rotationMatrix = getRotation().getRotationMatrix();
  auto translationMatrix =
      algebra::transformations::translationMatrix(getPosition());

  return translationMatrix * rotationMatrix * scaleMatrix;
}

void IEntity::rotateAroundPoint(const algebra::Quaternion<float> &rotation,
                                const algebra::Vec3f &point) {
  auto rotatedAtOrigin = rotation * (_position - point);
  _position = rotatedAtOrigin.toVector() + point;
  _rotation = rotation * _rotation;
}

void IEntity::scaleAroundPoint(float scaleFactor,
                               const algebra::Vec3f &centerPoint) {
  auto translatedPosition = _position - centerPoint;
  translatedPosition = translatedPosition * scaleFactor;

  _scale = _scale * scaleFactor;
  _position = translatedPosition + centerPoint;
}

const uint32_t &IEntity::getId() const { return _id; }
uint32_t &IEntity::getId() { return _id; }
const bool &IEntity::dirty() const { return _dirty; }
bool &IEntity::dirty() { return _dirty; }