#include "heightMapGenerator.hpp"
#include "bezierSurface.hpp"
#include "block.hpp"
#include "heightMap.hpp"
#include "vec.hpp"

static constexpr float kBaseHeight = 1.5f;
static constexpr uint32_t kDivisions = 3000;
static constexpr uint32_t kBaseDivisions = 1500;

HeightMap HeightMapGenerator::generateHeightMap(const Model &model,
                                                const Block &block) {
  HeightMap height_map(Divisions{.x_ = kBaseDivisions, .z_ = kBaseDivisions},
                       kBaseHeight, &block);

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

      auto surface_point = surface.value({u, v});

      if (surface_point.y() < 0.f) {
        continue;
      }

      auto height_map_index = heightMap.posToIndex(surface_point);
      if (surface_point.y() + kBaseHeight > heightMap.at(height_map_index)) {
        heightMap.at(height_map_index) = surface_point.y() + kBaseHeight;

        // here we may need to change normal orientation
        heightMap.normalAtIndex(height_map_index) = surface.normal({u, v});
      }
    }
  }
}
