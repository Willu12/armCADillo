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

  const Mesh &getMesh() const override { return _mesh; }
  const algebra::Vec3f &getPosition() const override { return _position; }

private:
  algebra::Vec3f _position = algebra::Vec3f(0.5f, 0.5f, 0.5f);
  float _radius = 0.05f;
  Mesh _mesh;

  Mesh generateMesh() {
    std::vector<float> vertices = {// left down
                                   -_radius, -_radius, 0.0f,
                                   // left up
                                   -_radius, _radius, 0.0f,
                                   // right up
                                   _radius, _radius, 0.f,
                                   // right down
                                   _radius, -_radius, 0.f};
    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

    return Mesh(vertices, indices);
  }
};