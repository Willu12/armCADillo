#pragma once

#include "vec.hpp"
class IDifferentialParametricForm {
public:
  virtual algebra::Vec2f bounds() const = 0;
  virtual algebra::Vec3f value(const algebra::Vec2f &pos) const = 0;
  virtual std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const = 0;
};