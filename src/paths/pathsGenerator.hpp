#pragma once

#include "bezierSurface.hpp"
#include "block.hpp"
#include "heightMapGenerator.hpp"
#include "millingPath.hpp"
#include "model.hpp"
#include <vector>
class PathsGenerator {
public:
  explicit PathsGenerator(std::vector<BezierSurface *> surfaces)
      : model_(std::move(surfaces)), heightMapGenerator_(&block_) {}

  MillingPath roughingPath();

private:
  Model model_;
  HeightMapGenerator heightMapGenerator_;
  Block block_ = Block::defaultBlock();
};