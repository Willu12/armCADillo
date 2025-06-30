#pragma once
#include "IDifferentialParametricForm.hpp"
#include "IEntity.hpp"
#include "intersectable.hpp"
#include "mesh.hpp"
#include "torus.hpp"
#include "vec.hpp"
#include <array>
#include <string>

class TorusEntity : public IEntity,
                    public algebra::IDifferentialParametricForm<2, 3>,
                    public Intersectable {
public:
  TorusEntity(float innerRadius, float tubeRadius, algebra::Vec3f position)
      : _torus(innerRadius, tubeRadius), _mesh(generateMesh()) {
    _position = position;
    _id = kClassId++;
    _name = ("Torus_" + std::to_string(TorusEntity::_id));
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

  std::array<algebra::Vec2f, 2> bounds() const override {
    return {algebra::Vec2f{0.f, 2.f * std::numbers::pi_v<float>},
            algebra::Vec2f{0.f, 2.f * std::numbers::pi_v<float>}};
  }
  algebra::Vec3f value(const algebra::Vec2f &pos) const override {
    auto affine =
        getModelMatrix() * (_torus.getPosition(pos[0], pos[1])).toHomogenous();
    return affine.fromHomogenous();
  }
  std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const override {
    auto R = getRotation().getRotationMatrix();
    auto derivatives = _torus.getDerivative(pos[0], pos[1]);
    algebra::Vec3f du_world =
        (R * derivatives.first.toHomogenous()).fromHomogenous();
    algebra::Vec3f dv_world =
        (R * derivatives.second.toHomogenous()).fromHomogenous();
    return {du_world, dv_world};
  }
  algebra::Matrix<float, 3, 2>
  jacobian(const algebra::Vec2f &pos) const override {
    auto [du, dv] = derivatives(pos);
    algebra::Matrix<float, 3, 2> J;
    for (int i = 0; i < 3; ++i) {
      J(i, 0) = du[i];
      J(i, 1) = dv[i];
    }

    return J;
  }

  bool wrapped(size_t dim) const override { return true; }

private:
  algebra::Torus<float> _torus;

  MeshDensity _meshDensity;
  std::shared_ptr<Mesh> _mesh;
  static inline int kClassId;

  std::shared_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrizationTextured(_torus, _meshDensity);
  }
};
