#pragma once
#include "IRenderable.hpp"
#include "mesh.hpp"
#include "sphere.hpp"

class Point : public IRenderable {
public:
  Point(algebra::Vec3f position)
      : _position(position), _sphere(0.01f), _mesh(generateMesh()) {}

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
  algebra::Vec3f &getPosition() override { return _position; }

protected:
  algebra::Sphere<float> _sphere;
  algebra::Vec3f _position;
  algebra::EulerAngle<float> _rotation;
  float _scale = 1.0f;
  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;

  std::shared_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrization(_sphere, _meshDensity);
  }
};