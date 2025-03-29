#pragma once

#include "IEntity.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "sphere.hpp"

class Point : public IEntity {
public:
  Point(algebra::Vec3f position)
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
    auto vertices = generateVertices();
    auto indices = generateIndices(vertices);

    return Mesh::create(vertices, indices);
  }

  // TO DO: MOVE IT TO MESH and add IPARAMETRIZABLE
  std::vector<float> generateVertices() {
    std::vector<float> vertices;
    for (int i = 0; i < _meshDensity.s; ++i) {
      float theta = i * (2.0f * M_PI / static_cast<float>(_meshDensity.s));
      for (int j = 0; j < _meshDensity.t; ++j) {
        float phi = j * (2.0f * M_PI / static_cast<float>(_meshDensity.t));

        const auto position = _sphere.getPosition(theta, phi).toVector();

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
        int right = j + ((i + 1) % _meshDensity.s) * _meshDensity.t;
        int down = i * _meshDensity.t + (j + 1) % _meshDensity.t;

        indices.push_back(current);
        indices.push_back(down);
        indices.push_back(current);
        indices.push_back(right);
      }
    }
    return indices;
  }
};