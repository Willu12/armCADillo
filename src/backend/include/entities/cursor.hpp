#pragma once
#include "IEntity.hpp"
#include "shader.hpp"
#include "vec.hpp"

class Cursor : public IEntity {
public:
  Cursor() : _mesh(generateMesh()) {}
  algebra::Mat4f getModelMatrix() const override {
    return algebra::transformations::translationMatrix(_position);
  }

  const Mesh &getMesh() const override { return *_mesh; }
  const algebra::Vec3f &getPosition() const override { return _position; }

private:
  algebra::Vec3f _position = algebra::Vec3f(0.5f, 0.5f, 0.5f);
  float _radius = 0.05f;
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