#pragma once
#include <concepts>

#include "../transformations.hpp"
#include "../vec.hpp"

namespace algebra {
template <std::floating_point T> class SphericalPosition {
public:
  T _r, _theta, _phi;
  SphericalPosition(T r, T theta, T phi) : _r(r), _theta(theta), _phi(phi) {}
  explicit SphericalPosition(algebra::Vec3<T> cartesianPosition) {
    _r = cartesianPosition.length();
    _theta = asin(cartesianPosition[1] / _r);
    _phi = atan(cartesianPosition[0] / cartesianPosition[1]);
  }

  algebra::Vec3<T> getCartesian() const {
    T x = _r * cos(_theta) * cos(_phi);
    T z = _r * cos(_theta) * sin(_phi);
    T y = _r * sin(_theta);

    return Vec3<T>(x, y, z);
  }

  algebra::Mat4<T> getRotationMatrix() const {
    return transformations::rotationXMatrix(_theta) *
           transformations::rotationYMatrix(_phi);
  }

private:
};
} // namespace algebra