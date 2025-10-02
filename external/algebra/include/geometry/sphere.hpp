#pragma once
#include "IParametrizable.hpp"
#include <concepts>

namespace algebra {
template <std::floating_point T> class Sphere : public IParametrizable<T> {
public:
  Sphere(T radius) : _radius(radius) {}

  T &getRadius() { return _radius; }

  std::vector<float> getBounds() const override { return {2 * M_PI, 2 * M_PI}; }

  Vec3<T> getPosition(T horizontalAngle, T verticalAngle) const override {
    T x = _radius * cos(verticalAngle) * cos(horizontalAngle);
    T z = _radius * cos(verticalAngle) * sin(horizontalAngle);
    T y = _radius * sin(verticalAngle);
    return Vec3<T>(x, y, z);
  }

private:
  T _radius;
};
} // namespace algebra