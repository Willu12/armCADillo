#pragma once

#include "../matrix.hpp"
#include "../vec.hpp"

namespace algebra {

template <size_t IN, size_t OUT> class IDifferentialParametricForm {
public:
  virtual ~IDifferentialParametricForm() = default;
  virtual bool wrapped(size_t dim) const = 0;
  virtual std::array<Vec2f, IN> bounds() const = 0;
  virtual Vec<float, OUT> value(const Vec<float, IN> &pos) const = 0;
  virtual std::pair<Vec<float, OUT>, Vec<float, OUT>>
  derivatives(const Vec<float, IN> &pos) const = 0;
  virtual Matrix<float, OUT, IN> jacobian(const Vec<float, IN> &pos) const = 0;
};
} // namespace algebra