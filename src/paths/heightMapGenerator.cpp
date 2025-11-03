#include "heightMapGenerator.hpp"
#include "IDifferentialParametricForm.hpp"
#include "bezierSurface.hpp"
#include "cutter.hpp"
#include "functions.hpp"
#include "gradientDescent.hpp"
#include "heightMap.hpp"
#include "newtonMethod.hpp"
#include "vec.hpp"
#include <vector>

static constexpr float kBaseHeight = 5.f;
static constexpr uint32_t kGridSearchSize = 10;
static constexpr uint32_t kBaseDivisions = 1500;

HeightMap HeightMapGenerator::generateHeightMap(const Model &model) {
  Cutter cutter{
      .type_ = Cutter::Type::Ball, .diameter_ = 1.6f, .height_ = 3.2f};

  HeightMap height_map(Divisions{.x_ = kBaseDivisions, .z_ = kBaseDivisions},
                       kBaseHeight);

  const auto zig_zag_points = generateZigZagPointsForCutter(cutter, height_map);
  for (const auto &point : zig_zag_points) {
    float height = geModelHeightInPosition(model, point);
  }
}

float HeightMapGenerator::geModelHeightInPosition(
    const Model &model, const algebra::Vec3f &position) const {
  float current_height = position.y();

  for (const auto *surface : model.surfaces()) {
  }
}

float HeightMapGenerator::getSurfaceHeightInPosition(
    const BezierSurface &surface, const algebra::Vec2f &position) const {
  /// we want to find (u,v) that x(u,v) = position.x() && y(u,v) =
  /// position.y()

  auto initial_uv =
      findInitialUVWithGridSearch(surface, position, kGridSearchSize);

  const auto function =
      std::make_unique<algebra::SurfacePointL2DistanceSquaredXZ>(&surface,
                                                                 position);
  // algebra::NewtonMethod<, size_t OUT>
}

std::vector<algebra::Vec3f> HeightMapGenerator::generateZigZagPointsForCutter(
    const Cutter &cutter, const HeightMap &heightMap) const {

  std::vector<algebra::Vec3f> points;
  points.reserve(heightMap.divisions().x_ * heightMap.divisions().z_);

  /// we create a zig zag pattern on XZ plane
  const auto dx =
      block_->dimensions_.x_ / static_cast<float>(heightMap.divisions().x_);
  const auto dz =
      block_->dimensions_.z_ / static_cast<float>(heightMap.divisions().z_);

  const auto half_x = block_->dimensions_.x_ / 2.f;
  const auto half_z = block_->dimensions_.z_ / 2.f;

  for (float x = -half_x; x < half_x; x += dx) {
    for (float z = -half_z; z < half_z; z += dz) {
      points.emplace_back(x, kBaseHeight, z);
    }
  }

  return points;
}

algebra::Vec2f HeightMapGenerator::findInitialUVWithGridSearch(
    const BezierSurface &surface, const algebra::Vec2f &positionXZ,
    uint32_t gridSize) const {
  auto bounds = surface.bounds();
  float min_dist_sq = std::numeric_limits<float>::max();
  algebra::Vec2f best_uv;

  auto update_best_dist = [&min_dist_sq, &best_uv](algebra::Vec2f currentUV,
                                                   float new_dist) {
    if (new_dist < min_dist_sq) {
      min_dist_sq = new_dist;
      best_uv = currentUV;
    }
  };

  for (int i = 0; i < gridSize; ++i) {
    for (int j = 0; j < gridSize; ++j) {
      float u = bounds[0][0] +
                (bounds[0][1] - bounds[0][0]) *
                    (static_cast<float>(i) / static_cast<float>(gridSize - 1));
      float v = bounds[1][0] +
                (bounds[1][1] - bounds[1][0]) *
                    (static_cast<float>(j) / static_cast<float>(gridSize - 1));

      auto p = surface.value({u, v});
      auto p_xz = algebra::Vec2f(p.x(), p.z());
      float dist_sq = std::pow((p_xz - positionXZ).length(), 2.f);

      update_best_dist(algebra::Vec2f(u, v), dist_sq);
    }
  }
  return best_uv;
}