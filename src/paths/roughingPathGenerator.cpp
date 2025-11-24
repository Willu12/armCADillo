#include "roughingPathGenerator.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "plane.hpp"
#include "rdp.hpp"
#include "vec.hpp"

static constexpr float kRDPEpsilon = 0.001f;

MillingPath RoughingPathGenerator::generate() {
  /// change later

  auto milling_points = calculateRoughMillingPoints();
  return MillingPath(std::move(milling_points), *cutter_);
}
void RoughingPathGenerator::setHeightMap(const HeightMap *heightMap) {
  heightMap_ = heightMap;
}
void RoughingPathGenerator::setCutter(const Cutter *cutter) {
  cutter_ = cutter;
}

std::vector<algebra::Vec3f>
RoughingPathGenerator::calculateRoughMillingPoints() const {
  std::vector<algebra::Vec3f> milling_points;

  const auto &block = heightMap_->block();
  const auto radius = cutter_->diameter_ / 2.f;
  const auto safe_offset = 1.2f * radius;

  const float left_x = -block.dimensions_.x_ / 2.f - safe_offset;
  const float right_x = block.dimensions_.x_ / 2.f + safe_offset;

  const float bottom_z = -block.dimensions_.z_ / 2.f - safe_offset;
  const float top_z = block.dimensions_.z_ / 2.f + safe_offset;
  const float safe_y = block.dimensions_.y_ + safe_offset;

  /// start at bottom left corner of the grid
  milling_points.emplace_back(left_x, safe_y, bottom_z);

  const auto dz = radius;
  const auto dz_pixels = static_cast<uint32_t>(
      dz * static_cast<float>(heightMap_->pixelCmRatio().first));

  const auto segments_count =
      static_cast<uint32_t>((block.dimensions_.z_ + safe_offset) / dz);

  const uint32_t x_divisions = heightMap_->divisions().x_;
  auto roughing_layer = [&](const float min_height) {
    for (int i = 0; i < segments_count; i++) {
      const bool forward = i % 2 == 0;
      uint32_t real_z = i * dz_pixels;

      std::vector<algebra::Vec3f> current_z_points;
      for (uint32_t x = 0; x < x_divisions; ++x) {
        uint32_t real_x = forward ? x : x_divisions - 1 - x;
        auto global_index = heightMap_->globalIndex(real_x, real_z);

        /// cut
        auto safe_cut_height = std::max(
            min_height,
            heightMap_->findMinimumSafeHeightForCut(global_index, *cutter_) +
                radius);

        /// add point only if height changes
        if (current_z_points.empty() ||
            safe_cut_height != current_z_points.back().y()) {
          auto point = heightMap_->indexToPos(global_index);
          current_z_points.emplace_back(point.x(), safe_cut_height, point.z());
        }
      }

      /// RDP reduction
      auto reduced_points = algebra::RDP::reducePoints(
          current_z_points, kRDPEpsilon, algebra::Plane::XY);
      milling_points.insert(milling_points.end(), reduced_points.begin(),
                            reduced_points.end());

      auto last_x = forward ? x_divisions - 1 : 0;
      auto pos =
          heightMap_->indexToPos(heightMap_->globalIndex(last_x, real_z));

      auto next_pos = heightMap_->indexToPos(heightMap_->globalIndex(
          last_x,
          static_cast<float>(i + 1) * dz * heightMap_->pixelCmRatio().first));

      milling_points.emplace_back(pos.x(), min_height, pos.z());
      milling_points.emplace_back(pos.x(), min_height, next_pos.z());
    }
  };

  /// first layer

  const auto max_y = block.dimensions_.y_;
  roughing_layer(max_y - cutter_->diameter_);
  milling_points.emplace_back(left_x, max_y - cutter_->diameter_, top_z);
  roughing_layer(max_y - 2.f * cutter_->diameter_);

  return milling_points;
}