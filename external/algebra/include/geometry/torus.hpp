#pragma once
#include "../vec.hpp"
#include "IParametrizable.hpp"
#include <concepts>

namespace algebra {
template <std::floating_point T> class Torus : public IParametrizable<T> {
public:
  Torus(T innerRadius, T tubeRadius)
      : _innerRadius(innerRadius), _tubeRadius(tubeRadius) {}

  T &getInnerRadius() { return _innerRadius; }
  T &getTubeRadius() { return _tubeRadius; }
  std::vector<float> getBounds() const override {
    return {2 * std::numbers::pi_v<float>, 2 * std::numbers::pi_v<float>};
  }

  Vec3<T> getPosition(T innerAngle, T tubeAngle) const override {
    T x = (_innerRadius + _tubeRadius * cos(tubeAngle)) * cos(innerAngle);
    T z = (_innerRadius + _tubeRadius * cos(tubeAngle)) * sin(innerAngle);
    T y = _tubeRadius * sin(tubeAngle);
    return Vec3<T>(x, y, z);
  }

  std::pair<Vec3<T>, Vec3<T>> getDerivative(T innerAngle, T tubeAngle) const {
    T cos_v = std::cos(tubeAngle);
    T sin_v = std::sin(tubeAngle);
    T cos_u = std::cos(innerAngle);
    T sin_u = std::sin(innerAngle);
    T common = _innerRadius + _tubeRadius * cos_v;

    Vec3<T> du{
        -common * sin_u,
        T(0),
        common * cos_u,
    };

    Vec3<T> dv{
        -_tubeRadius * sin_v * cos_u,
        _tubeRadius * cos_v,
        -_tubeRadius * sin_v * sin_u,
    };

    return {du, dv};
  }

private:
  T _innerRadius;
  T _tubeRadius;
};
} // namespace algebra