#pragma once
#include "IEntity.hpp"
#include "algebra.hpp"
#include "mesh.hpp"

struct MeshDensity {
  int s = 20;
  int t = 20;
};

class TorusModel : public IEntity {
public:
  TorusModel(float innerRadius, float tubeRadius, algebra::Vec3f position)
      : _torus(innerRadius, tubeRadius), _position(position),
        _mesh(generateMesh()) {}

  float &getInnerRadius() { return _torus.getInnerRadius(); }
  float &getTubeRadius() { return _torus.getTubeRadius(); }
  float &getScale() override { return _scale; }
  algebra::Vec3f &getPosition() override { return _position; }
  const algebra::Vec3f &getPosition() const override { return _position; }
  algebra::EulerAngle<float> &getRotation() override { return _rotation; }

  MeshDensity &getMeshDensity() { return _meshDensity; }

  algebra::Mat4f getModelMatrix() const override {
    auto scaleMatrix =
        algebra::transformations::scaleMatrix(_scale, _scale, _scale);
    auto rotationMatrix = _rotation.getRotationMatrix();
    auto translationMatrix =
        algebra::transformations::translationMatrix(_position);

    return translationMatrix * rotationMatrix * scaleMatrix;
  }

  inline const Mesh &getMesh() const override { return *_mesh; }

  void updateMesh() override { _mesh = generateMesh(); }

  bool renderSettings() override {
    bool change = false;
    change |= ImGui::SliderFloat("R", &getInnerRadius(), 0.1f, 10.f);
    change |= ImGui::SliderFloat("r", &getTubeRadius(), 0.1f, 10.f);
    change |=
        ImGui::SliderInt("Horizontal Density", &getMeshDensity().s, 3, 100);
    change |= ImGui::SliderInt("Vertical Density", &getMeshDensity().t, 3, 100);

    return change;
  }

private:
  algebra::Torus<float> _torus;
  algebra::Vec3f _position;
  algebra::EulerAngle<float> _rotation;
  float _scale = .1;
  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;

  std::shared_ptr<Mesh> generateMesh() {
    auto vertices = generateVertices();
    auto indices = generateIndices(vertices);

    return Mesh::create(vertices, indices);
  }

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