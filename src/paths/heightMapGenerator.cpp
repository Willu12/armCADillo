#include "heightMapGenerator.hpp"

HeightMap HeightMapGenerator::generateHeightMap(const Model &model) {
  return HeightMap{Divisions{.x_ = 0, .y_ = 0}, 5.f};
}