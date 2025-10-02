#pragma once
#include "concepts"

namespace algebra {
template <std::floating_point T> class Cube {
public:
  Cube(T a) : _a(a) {}

private:
  T _a;
};
} // namespace algebra