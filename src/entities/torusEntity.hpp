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
      : torus_(innerRadius, tubeRadius), mesh_(generateMesh()) {
    _position = position;
    _id = kClassId++;
    _name = ("Torus_" + std::to_string(TorusEntity::_id));
  }

  TorusEntity(float innerRadius, float tubeRadius, algebra::Vec3f position,
              const MeshDensity &meshDensity)
      : torus_(innerRadius, tubeRadius), meshDensity_(meshDensity),
        mesh_(generateMesh()) {
    _position = position;
    _id = kClassId++;
    _name = ("Torus_" + std::to_string(TorusEntity::_id));
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitTorus(*this);
  }

  float &getInnerRadius() { return torus_.getInnerRadius(); }
  float &getTubeRadius() { return torus_.getTubeRadius(); }

  MeshDensity &getMeshDensity() { return meshDensity_; }

  const Mesh &getMesh() const override { return *mesh_; }

  void updateMesh() override { mesh_ = generateMesh(); }

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
        getModelMatrix() * (torus_.getPosition(pos[0], pos[1])).toHomogenous();
    return affine.fromHomogenous();
  }

  std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const override {
    auto rotation_matrix = getRotation().getRotationMatrix();
    auto derivatives = torus_.getDerivative(pos[0], pos[1]);
    algebra::Vec3f du_world =
        (rotation_matrix * derivatives.first.toHomogenous()).fromHomogenous();
    algebra::Vec3f dv_world =
        (rotation_matrix * derivatives.second.toHomogenous()).fromHomogenous();
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
  algebra::Torus<float> torus_;
  MeshDensity meshDensity_;
  std::unique_ptr<Mesh> mesh_;

  static inline int kClassId;

  std::unique_ptr<Mesh> generateMesh() {
    return Mesh::fromParametrizationTextured(torus_, meshDensity_);
  }
};
