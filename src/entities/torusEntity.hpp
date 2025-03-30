#pragma once
#include "IEntity.hpp"
#include "algebra.hpp"
#include "imgui_stdlib.h"
#include "mesh.hpp"
#include <string>

class TorusEntity : public IEntity {
public:
  TorusEntity(float innerRadius, float tubeRadius, algebra::Vec3f position)
      : _torus(innerRadius, tubeRadius), _position(position),
        _mesh(generateMesh()),
        _name("Torus_" + std::to_string(TorusEntity::_id++)) {}

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

    if (ImGui::InputText("Name", &getName())) {
      change = true;
    }

    change |= ImGui::SliderFloat("R", &getInnerRadius(), 0.1f, 10.f);
    change |= ImGui::SliderFloat("r", &getTubeRadius(), 0.1f, 10.f);
    change |=
        ImGui::SliderInt("Horizontal Density", &getMeshDensity().s, 3, 100);
    change |= ImGui::SliderInt("Vertical Density", &getMeshDensity().t, 3, 100);

    return change;
  }

  std::string &getName() override { return _name; }

private:
  algebra::Torus<float> _torus;
  algebra::Vec3f _position;
  algebra::EulerAngle<float> _rotation;
  float _scale = .1;
  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;
  std::string _name;
  static inline int _id;

  std::shared_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrization(_torus, _meshDensity);
  }
};
