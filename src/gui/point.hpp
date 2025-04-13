#pragma once
#include "IEntity.hpp"
#include "sphere.hpp"

class Point : public IEntity {
public:
  Point(algebra::Vec3f position, float radius = 0.05f)
      : _sphere(radius), _mesh(generateMesh()) {
    _position = position;
  }

  const Mesh &getMesh() const override { return *_mesh; }
  void updateMesh() override { _mesh = generateMesh(); }
  bool renderSettings(const GUI &gui) override { return false; }

protected:
  algebra::Sphere<float> _sphere;

  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;

  std::shared_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrization(_sphere, _meshDensity);
  }
};