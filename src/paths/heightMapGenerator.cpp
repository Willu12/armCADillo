#include "heightMapGenerator.hpp"
#include "IDifferentialParametricForm.hpp"
#include "bezierSurface.hpp"
#include "block.hpp"
#include "cutter.hpp"
#include "functions.hpp"
#include "gradientDescent.hpp"
#include "heightMap.hpp"
#include "newtonMethod.hpp"
#include "vec.hpp"
#include <algorithm>
#include <vector>

static constexpr float kBaseHeight = 0.f;
static constexpr uint32_t kDivisions = 1000;
static constexpr uint32_t kBaseDivisions = 1500;

HeightMap HeightMapGenerator::generateHeightMap(const Model &model) {
  HeightMap height_map(Divisions{.x_ = kBaseDivisions, .z_ = kBaseDivisions},
                       kBaseHeight);

  for (const auto *surface : model.surfaces()) {
    processSurface(*surface, height_map);
  }

  return height_map;
}

void HeightMapGenerator::processSurface(const BezierSurface &surface,
                                        HeightMap &heightMap) const {

  const uint32_t u_divisions = kDivisions;
  const uint32_t v_divisions = kDivisions;

  for (uint32_t u_index = 0; u_index < u_divisions; ++u_index) {
    for (uint32_t v_index = 0; v_index < v_divisions; ++v_index) {
      float u = static_cast<float>(u_index) / static_cast<float>(u_divisions);
      float v = static_cast<float>(v_index) / static_cast<float>(v_divisions);

      auto surface_point = surface.value(algebra::Vec2f(u, v));
      auto height_map_index = getHeightMapIndex(surface_point, heightMap);

      heightMap.at(height_map_index) =
          std::max(heightMap.at(height_map_index), surface_point.y());
    }
  }
}

uint32_t
HeightMapGenerator::getHeightMapIndex(const algebra::Vec3f &pos,
                                      const HeightMap &heightMap) const {

  //// height map looks like
  /// Z|
  ///  |
  ///  |
  ///  |
  /// 0*---------
  ///  0       X

  auto get_index = [&](float x, float len, uint32_t divisions) {
    return static_cast<int>(static_cast<float>(divisions) *
                            ((x + len / 2.f) / len));
  };

  int x_index =
      get_index(pos.x(), block_->dimensions_.x_, heightMap.divisions().x_);
  int z_index =
      get_index(pos.z(), block_->dimensions_.z_, heightMap.divisions().z_);

  return z_index * heightMap.divisions().x_ + x_index;
}
