#pragma once
#include <concepts>
#include <vector>

namespace algebra {
template <std::floating_point T> class BernsteinPolynomial {
public:
  explicit BernsteinPolynomial(const std::vector<T> &coefficients)
      : _coefficients(coefficients) {}

  std::size_t degree() const { return _coefficients.size() - 1; }

  T value(T t) const {
    T t1 = 1.0 - t;
    auto values = _coefficients;

    for (int i = degree(); i > 0; --i)
      for (int j = 0; j < i; j++) {
        values[j] = values[j] * t1 + values[j + 1] * t;
      }
    return values[0];
  }
  std::vector<T> _coefficients;

private:
};
} // namespace algebra