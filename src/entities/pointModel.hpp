#pragma once

#include "IEntity.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "point.hpp"
#include "sphere.hpp"

class PointModel : public IEntity, public Point {
public:
  PointModel(algebra::Vec3f position)
      : Point(position), _name("Point_" + std::to_string(_id++)) {}

  algebra::Vec3f &getPosition() override { return _position; }

  algebra::EulerAngle<float> &getRotation() override { return _rotation; }
  float &getScale() override { return _scale; }
  void updateMesh() override { _mesh = generateMesh(); };

  std::string &getName() override { return _name; };

  // MAYBE CHANGE IT LATER???
  bool renderSettings() override { return false; }

  algebra::Mat4f getModelMatrix() const override {
    auto scaleMatrix =
        algebra::transformations::scaleMatrix(_scale, _scale, _scale);
    auto rotationMatrix = _rotation.getRotationMatrix();
    auto translationMatrix =
        algebra::transformations::translationMatrix(_position);

    return translationMatrix * rotationMatrix * scaleMatrix;
  }

  const Mesh &getMesh() const override { return *_mesh; }
  const algebra::Vec3f &getPosition() const override { return _position; }

private:
  std::string _name;
  inline static int _id;
};