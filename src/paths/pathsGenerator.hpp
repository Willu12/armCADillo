#pragma once

#include "GCodeSerializer.hpp"
#include "bezierSurface.hpp"
#include "block.hpp"
#include "flatPathGenerator.hpp"
#include "heightMap.hpp"
#include "heightMapGenerator.hpp"
#include "model.hpp"
#include "roughingPathGenerator.hpp"
#include <vector>

class PathsGenerator {
public:
  void setModel(const std::vector<BezierSurface *> &surfaces);
  void run();

private:
  std::unique_ptr<Model> model_ = nullptr;
  HeightMapGenerator heightMapGenerator_;
  GCodeSerializer gCodeSerializer_;
  Block block_ = Block::defaultBlock();

  /// Generators
  RoughingPathGenerator roughingPathGenerator_;
  FlatPathGenerator flatPathGenerator_;

  /// Helpers
};