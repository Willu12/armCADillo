#pragma once
#include "IEntity.hpp"
#include "mesh.hpp"
#include "torus.hpp"
#include <string>

class TorusEntity : public IEntity {
public:
  TorusEntity(float innerRadius, float tubeRadius, algebra::Vec3f position)
      : _torus(innerRadius, tubeRadius), _mesh(generateMesh()) {
    _position = position;
    _name = ("Torus_" + std::to_string(TorusEntity::_id++));
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitTorus(*this);
  }

  float &getInnerRadius() { return _torus.getInnerRadius(); }
  float &getTubeRadius() { return _torus.getTubeRadius(); }

  MeshDensity &getMeshDensity() { return _meshDensity; }

  const Mesh &getMesh() const override { return *_mesh; }

  void updateMesh() override { _mesh = generateMesh(); }

  bool renderSettings(const GUI &gui) override {
    IEntity::renderSettings(gui);
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

  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;
  static inline int _id;

  std::shared_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrization(_torus, _meshDensity);
  }
};
