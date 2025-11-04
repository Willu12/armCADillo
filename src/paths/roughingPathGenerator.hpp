#pragma once

#include "heightMap.hpp"
#include "millingPath.hpp"

class RoughingPathGenerator {
public:
  MillingPath generate(const HeightMap &heightMap);

private:
  std::vector<algebra::Vec3f>
  calculateRoughMillingPoints(const HeightMap &heightMap,
                              const Cutter &cutter) const;
};