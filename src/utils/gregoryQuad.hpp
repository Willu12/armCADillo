#pragma once

#include "vec.hpp"
#include <array>
struct GregoryQuad {
  std::array<algebra::Vec3f, 4> top;
  std::array<algebra::Vec3f, 4> bottom;
  std::array<algebra::Vec3f, 2> topSides;
  std::array<algebra::Vec3f, 2> bottomSides;
  std::array<algebra::Vec3f, 4> uInner;
  std::array<algebra::Vec3f, 4> vInner;
};