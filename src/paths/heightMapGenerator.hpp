#pragma once

#include "bezierSurface.hpp"
#include "block.hpp"
#include "heightMap.hpp"
#include "model.hpp"
#include "vec.hpp"

class HeightMapGenerator {
public:
  HeightMap generateHeightMap(const Model &model, const Block &block);

private:
  void processSurface(const BezierSurface &surface, HeightMap &heightMap) const;
};