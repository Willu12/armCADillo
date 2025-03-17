#pragma once
#include "algebra.hpp"
#include "mesh.hpp"

struct MeshDensity {
  unsigned int s = 10;
  unsigned int t = 10;
};

class TorusModel {
public:
  TorusModel(float innerRadius, float tubeRadius)
      : _torus(innerRadius, tubeRadius) {}
  Mesh generateMesh(const Shader &shader) {
    auto vertices = generateVertices();
    auto indices = generateIndices();

    return Mesh(vertices, indices, shader);
  }

private:
  algebra::Torus<float> _torus;
  algebra::Vec3f _position;
  algebra::EulerAngle<float> _rotation;
  MeshDensity _meshDensity;

  std::vector<float> generateVertices() {
    std::vector<float> vertices;
    for (int i = 0; i < _meshDensity.s; ++i) {
      float theta = i * (2.0f * M_PI / static_cast<float>(_meshDensity.s));
      for (int j = 0; j < _meshDensity.t; ++j) {
        float phi = j * (2.0f * M_PI / static_cast<float>(_meshDensity.t));

        const auto position = _torus.getPosition(theta, phi).toVector();

        vertices.insert(vertices.end(), position.begin(), position.end());
      }
    }
    return vertices;
  }
  std::vector<unsigned int> generateIndices() {
    std::vector<unsigned int> indices;
    for (int i = 0; i < _meshDensity.s; ++i) {
      for (int j = 0; j < _meshDensity.t; ++j) {
        int current = i * _meshDensity.t + j;
        int right = ((i + 1) % _meshDensity.s) * _meshDensity.t + j;
        int down = i * _meshDensity.t + (j + 1) % _meshDensity.t;

        indices.push_back(current);
        indices.push_back(right);
        indices.push_back(current);
        indices.push_back(down);
      }
    }
    return indices;
  }
};