#pragma once

#include "IEntity.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "sphere.hpp"

class PointModel : public IEntity {
public:
  PointModel(algebra::Vec3f position)
      : _position(position), _sphere(0.01f),
        _name("Point_" + std::to_string(_id++)), _mesh(generateMesh()) {}

  algebra::Vec3f &getPosition() override { return _position; }

  algebra::EulerAngle<float> &getRotation() override { return _rotation; }
  float &getScale() override { return _scale; }
  void updateMesh() override { _mesh = generateMesh(); };

  std::string &getName() override { return _name; };

  // MAYBE CHANGE IT LATER???
  bool renderSettings() override { return false; }

  // GET MODEL MATRIX SHOULD BE IN SOME ABSTRACTION
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
  algebra::Sphere<float> _sphere;
  algebra::Vec3f _position;
  algebra::EulerAngle<float> _rotation;
  float _scale = 1.f;
  std::string _name;
  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;
  inline static int _id;

  std::shared_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrization(_sphere, _meshDensity);
  }
};