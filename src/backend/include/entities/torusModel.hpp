#pragma once
#include "algebra.hpp"
#include "mesh.hpp"

struct MeshDensity {
  unsigned int s = 8;
  unsigned int t = 8;
};

class TorusModel {
public:
  TorusModel(float innerRadius, float tubeRadius)
      : _torus(innerRadius, tubeRadius) {}
  Mesh generateMesh(const Shader &shader) {
    auto vertices = generateVertices();
    auto indices = generateIndices(vertices);

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

        printf("[R_a =%f] , [r_a = %f]\n", theta, phi);
        const auto position = _torus.getPosition(theta, phi).toVector();

        vertices.insert(vertices.end(), position.begin(), position.end());
      }
    }
    return vertices;
  }
  std::vector<unsigned int>
  generateIndices(const std::vector<float> &vertices) {
    std::vector<unsigned int> indices;
    for (int i = 0; i < _meshDensity.s; ++i) {
      for (int j = 0; j < _meshDensity.t; ++j) {

        int current = i * _meshDensity.t + j;
        // int right = ((j + 1) % _meshDensity.t) * _meshDensity.t + i;
        // int down = j + _meshDensity.t * ((i + 1) % _meshDensity.s);
        int right = j + ((i + 1) % _meshDensity.s) * _meshDensity.t;
        int down = i * _meshDensity.t + (j + 1) % _meshDensity.t;

        printf("([%f, %f, %f], [%f, %f, %f]),\n", vertices[current * 3],
               vertices[current * 3 + 1], vertices[current * 3 + 2],
               vertices[down * 3], vertices[down * 3 + 1],
               vertices[down * 3 + 2]); //, current, down);
        indices.push_back(current);
        indices.push_back(down);
        // indices.push_back(current);

        // indices.push_back(down);
      }
    }
    return indices;
  }
};