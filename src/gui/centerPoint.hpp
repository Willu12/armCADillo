#pragma once
#include "IEntity.hpp"
#include "point.hpp"

class CenterPoint {
public:
  CenterPoint() : _point(algebra::Vec3f()) {}
  void display(const std::vector<IEntity *> &entities) {
    if (entities.size() < 2)
      return;
    updatePosition(entities);
  }

  const IRenderable &getPoint() { return _point; }

private:
  Point _point;

  void updatePosition(const std::vector<IEntity *> &entities) {
    auto averagePosition = algebra::Vec3f();

    for (const auto &entity : entities) {
      averagePosition = averagePosition + entity->getPosition();
    }
    averagePosition = averagePosition / static_cast<float>(entities.size());

    auto &pointPosition = _point.getPosition();
    pointPosition = averagePosition;
  }
};