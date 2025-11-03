#pragma once

#include "bezierSurface.hpp"
#include "block.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "model.hpp"
#include "vec.hpp"

class HeightMapGenerator {
public:
  explicit HeightMapGenerator(Block *block) : block_(block) {}

  HeightMap generateHeightMap(const Model &model);

private:
  Block *block_ = nullptr;

  ///
  float geModelHeightInPosition(const Model &model,
                                const algebra::Vec3f &position) const;

  float getSurfaceHeightInPosition(const BezierSurface &surface,
                                   const algebra::Vec2f &positionXZ) const;

  algebra::Vec2f findInitialUVWithGridSearch(const BezierSurface &surface,
                                             const algebra::Vec2f &positionXZ,
                                             uint32_t gridSize) const;

  ///
  std::vector<algebra::Vec3f>
  generateZigZagPointsForCutter(const Cutter &cutter,
                                const HeightMap &heightMap) const;
};