#pragma once
#include "IEntity.hpp"

class Point : public IEntity {
public:
  explicit Point(algebra::Vec3f position, float radius = 0.01f)
      : _radius(radius), _mesh(generateMesh()) {
    _position = position;
  }

  const Mesh &getMesh() const override { return *_mesh; }
  void updateMesh() override { _mesh = generateMesh(); }
  bool renderSettings(const GUI &gui) override { return false; }
  uint32_t getId() const override { return 0; }

protected:
  float _radius;
  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;

  std::shared_ptr<Mesh> generateMesh() {
    std::vector<float> vertices = {// left down             //text
                                   -_radius, -_radius, 0.f, 0.f, 0.f,
                                   // left up               //text
                                   -_radius, _radius, 0.f, 0.f, 1.0,
                                   // right up             //text
                                   _radius, _radius, 0.f, 1.f, 1.f,
                                   // right down             //text
                                   _radius, -_radius, 0.f, 1.f, 0.f};
    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

    return Mesh::createTexturedMesh(vertices, indices);
  }
};