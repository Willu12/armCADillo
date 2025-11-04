#pragma once

#include "GCodeSerializer.hpp"
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
  void setModel(const std::vector<BezierSurface *> &surfaces);
  void run();

private:
  std::unique_ptr<Model> model_ = nullptr;
  HeightMapGenerator heightMapGenerator_;
  GCodeSerializer gCodeSerializer_;

  Block block_ = Block::defaultBlock();

  MillingPath roughingPath();

  std::vector<algebra::Vec3f>
  calculateRoughMillingPoints(const HeightMap &heightMap,
                              const Cutter &cutter) const;
};