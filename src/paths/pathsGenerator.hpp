#pragma once

#include "GCodeSerializer.hpp"
#include "bezierSurface.hpp"
#include "block.hpp"
#include "detailedPathGenerator.hpp"
#include "flatPathGenerator.hpp"
#include "heightMap.hpp"
#include "heightMapGenerator.hpp"
#include "intersectionFinder.hpp"
#include "model.hpp"
#include "roughingPathGenerator.hpp"
#include <memory>
#include <vector>

class PathsGenerator {
public:
  void run();
  void setIntersectionFinder(IntersectionFinder *intersectionFinder);
  void setModel(const std::vector<BezierSurface *> &surfaces);
  void setScene(Scene *scene);

  const HeightMap *heightMap() const { return heightMap_.get(); }

private:
  std::unique_ptr<Model> model_ = nullptr;
  HeightMapGenerator heightMapGenerator_;
  GCodeSerializer gCodeSerializer_;
  Block block_ = Block::defaultBlock();

  std::unique_ptr<HeightMap> heightMap_ = nullptr;

  /// Generators
  RoughingPathGenerator roughingPathGenerator_;
  FlatPathGenerator flatPathGenerator_;
  DetailedPathGenerator detailedPathGenerator_;

  /// Helpers
};