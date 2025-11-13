#pragma once
#include "../parametricForms/IDifferentialParametricForm.hpp"
#include "surface.hpp"

namespace algebra {
class NormalOffsetSurface : public IDifferentialParametricForm<2, 3> {
public:
  explicit NormalOffsetSurface(const BezierSurfaceC0 *surface, float offset)
      : surface_(surface), offset_(offset) {}

  bool wrapped(size_t dim) const override { return surface_->wrapped(dim); }

  std::array<Vec2f, 2> bounds() const override { return surface_->bounds(); }

  Vec<float, 3> value(const Vec2f &pos) const override {
    return surface_->value(pos) + offset_ * surface_->normal(pos);
  }

  std::pair<Vec3f, Vec3f> derivatives(const Vec2f &pos) const override {
    auto [s_u, s_v] = surface_->derivatives(pos);
    auto second = surface_->secondDerivatives(pos);

    Vec3f a = s_u.cross(s_v);
    float a_len = a.length();

    if (a_len < std::numeric_limits<float>::epsilon()) {
      return {s_u, s_v};
    }

    Vec3f n = a / a_len;

    Vec3f a_u = second.duu.cross(s_v) + s_u.cross(second.duv);
    Vec3f a_v = second.duv.cross(s_v) + s_u.cross(second.dvv);

    Vec3f n_u = (a_u - n * n.dot(a_u)) / a_len;
    Vec3f n_v = (a_v - n * n.dot(a_v)) / a_len;

    Vec3f s_u_prime = s_u + offset_ * n_u;
    Vec3f s_v_prime = s_v + offset_ * n_v;

    return {s_u_prime, s_v_prime};
  }

  Matrix<float, 3, 2> jacobian(const Vec2f &pos) const override {
    auto [du, dv] = derivatives(pos);

    algebra::Matrix<float, 3, 2> jacobian;
    jacobian(0, 0) = du[0];
    jacobian(1, 0) = du[1];
    jacobian(2, 0) = du[2];

    jacobian(0, 1) = dv[0];
    jacobian(1, 1) = dv[1];
    jacobian(2, 1) = dv[2];

    return jacobian;
  }

  const BezierSurfaceC0 *baseSurface() const { return surface_; }

private:
  const BezierSurfaceC0 *surface_;
  float offset_;
};
} // namespace algebra