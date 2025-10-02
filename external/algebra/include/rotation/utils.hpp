#pragma once
#include <cmath>
#include <concepts>
#include <numbers>

namespace algebra {
namespace rotations {
template <std::floating_point T> static T toRadians(T angle) {
  return angle * std::numbers::pi_v<T> / 180.0;
}
} // namespace rotations
} // namespace algebra