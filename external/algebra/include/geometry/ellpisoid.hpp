#pragma once

#include "../matrix.hpp"
#include <concepts>

namespace algebra {
template <std::floating_point T> class Ellipsoid {
public:
  Ellipsoid(T a, T b, T c) : a(a), b(b), c(c) {}
  Mat4<T> toMatrix() {
    std::array<std::array<T, 4>, 4> rows = {{{1.0f / (powf(a, 2)), 0, 0, 0},
                                             {0, 1.0f / (powf(b, 2)), 0, 0},
                                             {0, 0, 1.0f / (powf(c, 2)), 0},
                                             {0, 0, 0, -1}}};
    return Mat4<T>::fromRows(rows);
  }

  T a, b, c;
};
} // namespace algebra