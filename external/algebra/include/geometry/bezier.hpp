#pragma once
#include "../polynomials/bernsteinPolynomial.hpp"
#include "../vec.hpp"
#include <concepts>
#include <vector>

namespace algebra {
template <std::floating_point T> class BezierCurve {
public:
  explicit BezierCurve(const Vec3<T> &points) {
    std::vector<T> x_coeffs, y_coeffs, z_coeffs;
    for (const auto &point : points) {
      x_coeffs.push_back(point[0]);
      y_coeffs.push_back(point[1]);
      z_coeffs.push_back(point[2]);
    }
    _x = BernsteinPolynomial<T>(x_coeffs);
    _y = BernsteinPolynomial<T>(y_coeffs);
    _z = BernsteinPolynomial<T>(z_coeffs);
  }

  std::vector<Vec3<T>> points() const {
    std::vector<Vec3<T>> points;
    for (int i = 0; i < _x.degree(); i++)
      points.push_back(Vec3<T>(_x._coefficients[i], _y._coefficients[i],
                               _z._coefficients[i]));

    return points;
  }

  Vec3<T> value(T t) const {
    return Vec3<T>(_x.value(t), _y.value(t), _z.value(t));
  }

  std::vector<T> getBounds() const { return {0.0, 1.0}; }

private:
  algebra::BernsteinPolynomial<T> _x, _y, _z;
};
} // namespace algebra
