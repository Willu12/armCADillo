#pragma once

#include "bezierSurface.hpp"
#include "heightMapGenerator.hpp"
#include "millingPath.hpp"
#include "model.hpp"
#include <vector>
class PathsGenerator {
public:
  explicit PathsGenerator(std::vector<BezierSurface *> surfaces)
      : model_(std::move(surfaces)) {}

  MillingPath roughingPath();

private:
  Model model_;
  HeightMapGenerator heightMapGenerator_;
};