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

  const float bottom_z = -block.dimensions_.z_ / 2.f - safe_offset;
  const float block_start_z = -block.dimensions_.z_ / 2.f;

  const float left_x = -block.dimensions_.x_ / 2.f - safe_offset;
  const float safe_y = block.dimensions_.y_ + safe_offset;

  milling_points.emplace_back(left_x, safe_y, bottom_z);

  const auto dz = radius;
  const auto pixel_ratio = static_cast<float>(heightMap_->pixelCmRatio().first);
  const auto dz_pixels = static_cast<uint32_t>(dz * pixel_ratio);

  const auto segments_count =
      static_cast<int>((block.dimensions_.z_ + 2.0f * safe_offset) / dz);

  const uint32_t x_divisions = heightMap_->divisions().x_;
  const uint32_t z_divisions = heightMap_->divisions().z_;

  auto roughing_layer = [&](const float min_height, bool reverse_layer) {
    int start_i = reverse_layer ? segments_count - 1 : 0;
    int end_i = reverse_layer ? -1 : segments_count;
    int step = reverse_layer ? -1 : 1;

    for (int i = start_i; i != end_i; i += step) {
      const bool forward_x = (i % 2 == 0);
      float current_physical_z = bottom_z + (static_cast<float>(i) * dz);

      int raw_z_index =
          static_cast<int>((current_physical_z - block_start_z) * pixel_ratio);
      uint32_t clamped_z_index =
          std::clamp(raw_z_index, 0, (int)z_divisions - 1);

      std::vector<algebra::Vec3f> current_z_points;

      for (uint32_t x = 0; x < x_divisions; ++x) {
        uint32_t real_x = forward_x ? x : x_divisions - 1 - x;
        auto global_index = heightMap_->globalIndex(real_x, clamped_z_index);

        float cut_height = heightMap_->findMinimumSafeHeightForCut(
            global_index, *cutter_); // +
                                     //   radius;

        if (raw_z_index < 0 || raw_z_index >= static_cast<int>(z_divisions)) {
          cut_height = min_height;
        }

        auto safe_cut_height = std::max(min_height, cut_height);

        float pos_x = heightMap_->indexToPos(global_index).x();

        if (current_z_points.empty() ||
            safe_cut_height != current_z_points.back().y()) {
          current_z_points.emplace_back(pos_x, safe_cut_height,
                                        current_physical_z);
        }
      }

      auto reduced_points = algebra::RDP::reducePoints(
          current_z_points, kRDPEpsilon, algebra::Plane::XY);
      milling_points.insert(milling_points.end(), reduced_points.begin(),
                            reduced_points.end());

      bool has_next_strip = reverse_layer ? (i > 0) : (i < segments_count - 1);

      if (has_next_strip) {
        auto last_x_index = forward_x ? x_divisions - 1 : 0;
        float current_x_pos =
            heightMap_->indexToPos(heightMap_->globalIndex(last_x_index, 0))
                .x();
        float next_physical_z = bottom_z + (static_cast<float>(i + step) * dz);

        milling_points.emplace_back(current_x_pos, min_height,
                                    current_physical_z);
        milling_points.emplace_back(current_x_pos, min_height, next_physical_z);
      }
    }
  };

  const auto max_y = block.dimensions_.y_;
  roughing_layer(max_y - cutter_->diameter_, false);
  roughing_layer(max_y - 2.f * cutter_->diameter_, true);

  return milling_points;
}