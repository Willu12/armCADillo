#pragma once

#include <vector>

namespace algebra {

class Polynomial {};

template <std::floating_point T> class Quadratic : public Polynomial {
public:
  Quadratic(T a, T b, T c) : a(a), b(b), c(c) {}

  inline std::vector<T> solve() {
    std::vector<T> solutions;
    T delta = b * b - 4 * a * c;
    if (delta < 0) {
      return solutions;
    }
    if (delta == 0) {
      solutions.push_back(-b / (2 * a));
      return solutions;
    }
    solutions.push_back((-b + sqrt(delta)) / (2 * a));
    solutions.push_back((-b - sqrt(delta)) / (2 * a));
    return solutions;
  }

private:
  T a, b, c;
};

} // namespace algebra