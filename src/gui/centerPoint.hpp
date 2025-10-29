#pragma once
#include "IEntity.hpp"
#include "point.hpp"

class CenterPoint {
public:
  CenterPoint() : _point(algebra::Vec3f()) {}
  void display(const std::vector<IEntity *> &entities) {
    if (entities.size() < 1) {
      return;
    }
    updatePosition(entities);
  }

  const algebra::Vec3f &getPosition() const { return _point.getPosition(); }
  const IRenderable &getPoint() { return _point; }

private:
  Point _point;

  void updatePosition(const std::vector<IEntity *> &entities) {
    auto average_position = algebra::Vec3f();

    for (const auto &entity : entities) {
      average_position = average_position + entity->getPosition();
    }
    average_position = average_position / static_cast<float>(entities.size());

    _point.updatePosition(average_position);
  }
};
