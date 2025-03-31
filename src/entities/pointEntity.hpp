#pragma once

#include "IEntity.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "point.hpp"
#include "sphere.hpp"

class PointEntity : public Point {
public:
  PointEntity(algebra::Vec3f position) : Point(position, 0.1f) {
    _name = ("Point_" + std::to_string(_id++));
  }

  void updateMesh() override { _mesh = generateMesh(); };
  bool renderSettings() override { return false; }
  const Mesh &getMesh() const override { return *_mesh; }

private:
  inline static int _id;
};