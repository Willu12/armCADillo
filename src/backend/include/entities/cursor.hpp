#pragma once
#include "shader.hpp"
#include "vec.hpp"

class Cursor {
public:
  Mesh generateMesh(Shader &shader) {
    std::vector<float> vertices = {// center
                                   0.0f, 0.0f, 0.0f,
                                   // right
                                   _radius, 0.0f, 0.0f,
                                   // up
                                   0.0f, _radius, 0.0f,
                                   // left
                                   -_radius, 0.0f, 0.f,
                                   // down
                                   0.f, -_radius, 0.f};
    std::vector<unsigned int> indices = {0, 1, 0, 2, 0, 3, 0, 4};

    return Mesh(vertices, indices, shader);
  }

  algebra::Mat4f getModelMatrix() const {
    return algebra::transformations::translationMatrix(_position);
  }

private:
  algebra::Vec3f _position;
  float _radius = 0.05f;
};