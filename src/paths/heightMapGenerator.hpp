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
  void processSurface(const BezierSurface &surface, HeightMap &heightMap) const;
  uint32_t getHeightMapIndex(const algebra::Vec3f &pos,
                             const HeightMap &heightMap) const;
};