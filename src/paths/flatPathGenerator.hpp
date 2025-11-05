#pragma once

#include "heightMap.hpp"
#include "millingPath.hpp"
#include "vec.hpp"
#include <cstdint>
#include <vector>

class FlatPathGenerator {
public:
  MillingPath generate(HeightMap &heightMap) const;

private:
  std::vector<uint32_t> findBoundaryIndices(HeightMap &heightMap) const;
  std::vector<algebra::Vec3f>
  findCutterPositionsFromBoundary(const HeightMap &heightMap,
                                  const std::vector<uint32_t> &boundaryIndices,
                                  const Cutter &cutter) const;

  /// helperFunction
  void paintBorderRed(HeightMap &heightMap,
                      const std::vector<uint32_t> &boundaryIndices) const;
};