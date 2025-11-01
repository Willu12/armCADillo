#include "heightMapGenerator.hpp"
#include "heightMap.hpp"

static constexpr float kBaseHeight = 5.f;
static constexpr uint32_t kBaseDivisions = 1500;

HeightMap HeightMapGenerator::generateHeightMap(const Model &model) {

  HeightMap height_map(Divisions{.x_ = kBaseDivisions, .y_ = kBaseDivisions},
                       kBaseHeight);
  // return HeightMap{Divisions{.x_ = 0, .y_ = 0}, 5.f};
}