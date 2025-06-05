#pragma once

#include "bezierSurface.hpp"
#include "vec.hpp"
#include <functional>
#include <vector>
class GregorySurface : public BezierSurface {
public:
  explicit GregorySurface(
      const std::vector<std::reference_wrapper<BezierSurface>> &surfaces) {}

private:
  std::vector<std::array<algebra::Vec3f, 16>> calculateGregoryPoints();
};