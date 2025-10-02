#pragma once
#include "../vec.hpp"
#include <vector>

namespace algebra {
template <std::floating_point T> class IParametrizable {
public:
  virtual ~IParametrizable() = default;
  virtual std::vector<float> getBounds() const = 0;
  virtual Vec3<T> getPosition(T theta, T phi) const = 0;
};
} // namespace algebra