#pragma once

#include "matrix.hpp"
#include "parametricForms/IDifferentialParametricForm.hpp"
#include "vec.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace algebra {
template <size_t SIZE> class IDifferentiableScalarFunction {
public:
  virtual ~IDifferentiableScalarFunction() = default;
  virtual bool wrapped(size_t dim) const = 0;
  virtual std::array<Vec2f, SIZE> bounds() const = 0;
  virtual float value(const Vec<float, SIZE> &x) const = 0;
  virtual Vec<float, SIZE> gradient(const Vec<float, SIZE> &x) const = 0;
  virtual bool same() const = 0;

private:
};

class SurfacePointL2DistanceSquared : public IDifferentiableScalarFunction<2> {
public:
  SurfacePointL2DistanceSquared(
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface,
      const Vec3f &point)
      : surface_(std::move(surface)), point_(point) {}

  bool wrapped(size_t dim) const override {
    return surface_.lock()->wrapped(dim);
  }
  std::array<Vec2f, 2> bounds() const override {
    return surface_.lock()->bounds();
  }

  float value(const Vec<float, 2> &x) const override {
    return std::pow((surface_.lock()->value(x) - point_).length(), 2.f);
  }
  Vec<float, 2> gradient(const Vec<float, 2> &x) const override {
    auto surface = surface_.lock();
    if (!surface) {
      return {0.f, 0.f};
    }

    Vec3f S = surface->value(x);
    Vec3f diff = S - point_;

    auto [Su, Sv] = surface->derivatives(x);

    float grad_u = 2.0f * Su.dot(diff);
    float grad_v = 2.0f * Sv.dot(diff);

    return {grad_u, grad_v};
  };
  bool same() const override { return false; }

private:
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface_;
  Vec3f point_;
};

class SurfaceSurfaceL2DistanceSquared
    : public IDifferentiableScalarFunction<4> {
public:
  SurfaceSurfaceL2DistanceSquared(
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface0,
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface1)
      : surface0_(std::move(surface0)), surface1_(std::move(surface1)) {}

  std::array<Vec2f, 4> bounds() const override {
    auto bounds0 = surface0_.lock()->bounds();
    auto bounds1 = surface1_.lock()->bounds();

    return {bounds0[0], bounds0[1], bounds1[0], bounds1[1]};
  }

  bool wrapped(size_t dim) const override {
    if (dim == 0 || dim == 1) {
      return surface0_.lock()->wrapped(dim);
    }
    if (dim == 2 || dim == 3) {
      return surface1_.lock()->wrapped(dim - 2);
    }

    return false;
  }
  float value(const Vec<float, 4> &x) const override {
    auto val1 = surface0_.lock()->value(Vec2f(x[0], x[1]));
    auto val2 = surface1_.lock()->value(Vec2f(x[2], x[3]));
    return std::pow((val1 - val2).length(), 2.f);
  }

  Vec<float, 4> gradient(const Vec<float, 4> &x) const override {
    auto s0 = surface0_.lock();
    auto s1 = surface1_.lock();
    if (!s0 || !s1) {
      return Vec<float, 4>();
    }

    Vec2f uv0{x[0], x[1]};
    Vec2f uv1{x[2], x[3]};

    Vec3f p0 = s0->value(uv0);
    Vec3f p1 = s1->value(uv1);
    Vec3f delta = p0 - p1;

    auto [dp0_du, dp0_dv] = s0->derivatives(uv0);
    auto [dp1_du, dp1_dv] = s1->derivatives(uv1);

    float df_du0 = 2.0f * dp0_du.dot(delta);
    float df_dv0 = 2.0f * dp0_dv.dot(delta);
    float df_du1 = -2.0f * dp1_du.dot(delta);
    float df_dv1 = -2.0f * dp1_dv.dot(delta);

    return Vec<float, 4>{df_du0, df_dv0, df_du1, df_dv1};
  }

  bool same() const override {
    return surface0_.lock().get() == surface1_.lock().get();
  }

private:
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface0_;
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface1_;
};

class IntersectionStepFunction : public IDifferentialParametricForm<4, 4> {
public:
  IntersectionStepFunction(
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface0,
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface1,
      const Vec3f &lastCommonPoint, const Vec3f &dir)
      : surface0_(surface0), surface1_(surface1),
        lastCommonPoint_(lastCommonPoint), direction_(dir) {}

  void setStep(float step) { step_ = step; }
  inline std::array<Vec2f, 4> bounds() const override {
    auto bounds0 = surface0_.lock()->bounds();
    auto bounds1 = surface1_.lock()->bounds();

    return {bounds0[0], bounds0[1], bounds1[0], bounds1[1]};
  }
  inline Vec4f value(const Vec4f &pos) const override {
    auto surf0Val = surface0_.lock()->value(Vec2f(pos[0], pos[1]));
    auto surf1Val = surface1_.lock()->value(Vec2f(pos[2], pos[3]));
    auto surfDiff = surf0Val - surf1Val;
    auto midPoint = (surf0Val + surf1Val) / 2.f;

    auto displacementProjectionLength =
        (midPoint - lastCommonPoint_).dot(direction_) - step_;

    return algebra::Vec4f{surfDiff[0], surfDiff[1], surfDiff[2],
                          displacementProjectionLength};
  }

  inline bool wrapped(size_t dim) const override {
    if (dim == 0 || dim == 1)
      return surface0_.lock()->wrapped(dim);
    if (dim == 2 || dim == 3)
      return surface1_.lock()->wrapped(dim - 2);

    return false;
  }
  inline std::pair<Vec4f, Vec4f> derivatives(const Vec4f &pos) const override {
    throw std::runtime_error("NOT Implemented");
  }
  inline Mat4f jacobian(const Vec4f &pos) const override {
    Vec2f uv0{pos[0], pos[1]};
    Vec2f uv1{pos[2], pos[3]};

    auto surf0 = surface0_.lock();
    auto surf1 = surface1_.lock();

    auto [du0, dv0] = surf0->derivatives(uv0); // ∂p0/∂u0, ∂p0/∂v0
    auto [du1, dv1] = surf1->derivatives(uv1); // ∂p1/∂u1, ∂p1/∂v1

    Mat4f J;

    for (int i = 0; i < 3; ++i) {
      J(i, 0) = du0[i];
      J(i, 1) = dv0[i];
      J(i, 2) = -du1[i];
      J(i, 3) = -dv1[i];
    }

    Vec3f dir = direction_;
    J(3, 0) = 0.5f * dir.dot(du0);
    J(3, 1) = 0.5f * dir.dot(dv0);
    J(3, 2) = 0.5f * dir.dot(du1);
    J(3, 3) = 0.5f * dir.dot(dv1);

    return J;
  }

private:
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface0_;
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface1_;
  Vec3f lastCommonPoint_;
  Vec3f direction_;
  float step_ = 0.05f;
};

class IntersectionFunction : public IDifferentialParametricForm<4, 4> {
public:
  IntersectionFunction(
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface0,
      std::weak_ptr<IDifferentialParametricForm<2, 3>> surface1)
      : surface0_(surface0), surface1_(surface1) {}

  inline std::array<Vec2f, 4> bounds() const override {
    auto bounds0 = surface0_.lock()->bounds();
    auto bounds1 = surface1_.lock()->bounds();
    return {bounds0[0], bounds0[1], bounds1[0], bounds1[1]};
  }

  inline Vec4f value(const Vec4f &pos) const override {
    auto surf0Val = surface0_.lock()->value(Vec2f(pos[0], pos[1]));
    auto surf1Val = surface1_.lock()->value(Vec2f(pos[2], pos[3]));
    Vec3f diff = surf0Val - surf1Val;

    auto J = getExactJacobian(pos);
    auto JT = J.transpose();

    return (JT * Matrix<float, 3, 1>::fromVector(diff)).toVector();
  }

  inline bool wrapped(size_t dim) const override {
    if (dim < 2)
      return surface0_.lock()->wrapped(dim);
    else
      return surface1_.lock()->wrapped(dim - 2);
  }

  inline std::pair<Vec4f, Vec4f> derivatives(const Vec4f &pos) const override {
    throw std::runtime_error("NOT Implemented");
  }

  inline Matrix<float, 4, 4> jacobian(const Vec4f &pos) const override {
    auto J = getExactJacobian(pos);

    return J.transpose() * J;
  }

private:
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface0_;
  std::weak_ptr<IDifferentialParametricForm<2, 3>> surface1_;

  inline Matrix<float, 3, 4> getExactJacobian(const Vec4f &pos) const {
    Vec2f uv0{pos[0], pos[1]};
    Vec2f uv1{pos[2], pos[3]};

    auto surf0 = surface0_.lock();
    auto surf1 = surface1_.lock();

    auto [du0, dv0] = surf0->derivatives(uv0);
    auto [du1, dv1] = surf1->derivatives(uv1);

    Vec3f diff = surf0->value(uv0) - surf1->value(uv1);

    Matrix<float, 3, 4> J;

    for (int coord = 0; coord < 3; ++coord) {
      J(coord, 0) = du0[coord];
      J(coord, 1) = dv0[coord];
      J(coord, 2) = -du1[coord];
      J(coord, 3) = -dv1[coord];
    }

    return J;
  }
};

} // namespace algebra