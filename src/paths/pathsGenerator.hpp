#pragma once

#include "bezierSurface.hpp"
#include "block.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "heightMapGenerator.hpp"
#include "millingPath.hpp"
#include "model.hpp"
#include "vec.hpp"
#include <vector>
class PathsGenerator {
public:
  explicit PathsGenerator(std::vector<BezierSurface *> surfaces)
      : model_(std::move(surfaces)) {}

  MillingPath roughingPath();

private:
  Model model_;
  HeightMapGenerator heightMapGenerator_;
  Block block_ = Block::defaultBlock();

  std::vector<algebra::Vec3f>
  calculateRoughMillingPoints(const HeightMap &heightMap,
                              const Cutter &cutter) const;
};