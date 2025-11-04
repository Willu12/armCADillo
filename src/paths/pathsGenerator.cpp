#include "pathsGenerator.hpp"
#include "cutter.hpp"
#include "millingPath.hpp"
#include "model.hpp"
#include "vec.hpp"

void PathsGenerator::setModel(const std::vector<BezierSurface *> &surfaces) {
  model_ = std::make_unique<Model>(surfaces);
}

MillingPath PathsGenerator::roughingPath() {
  /// change later
  Cutter cutter{
      .type_ = Cutter::Type::Ball,
      .diameter_ = 1.6f,
      .height_ = 2.f * 1.6f,
  };

  auto height_map = heightMapGenerator_.generateHeightMap(*model_, block_);
  auto milling_points = calculateRoughMillingPoints(height_map, cutter);
  return MillingPath(std::move(milling_points), cutter);
};

std::vector<algebra::Vec3f>
PathsGenerator::calculateRoughMillingPoints(const HeightMap &heightMap,
                                            const Cutter &cutter) const {
  std::vector<algebra::Vec3f> milling_points;

  const auto &block = heightMap.block();
  const auto safe_offset = cutter.diameter_;

  /// we start at bottom left corner of the grid
  auto starting_point =
      algebra::Vec3f(-block.dimensions_.x_ / 2.f - safe_offset,
                     block.dimensions_.y_ + safe_offset,
                     -block.dimensions_.z_ / 2.f - safe_offset);
  milling_points.push_back(starting_point);

  /// we would like to offset cutter
  const auto min_height = block.dimensions_.y_ - cutter.height_;
  const auto dz = cutter.diameter_;

  for (int z = 0; static_cast<float>(z) < block_.dimensions_.z_ / dz; z++) {
    const bool left_to_right = z % 2 == 0;

    uint32_t real_z = z * dz * heightMap.pixelCmRatio().first;
    for (uint32_t x = 0; x < heightMap.divisions().x_; ++x) {
      uint32_t real_x = left_to_right ? x : heightMap.divisions().x_ - 1 - x;
      auto global_index = heightMap.globalIndex(real_x, real_z);

      /// cut
      auto safe_cut_height =
          std::max(min_height,
                   heightMap.findMinimumSafeHeightForCut(global_index, cutter));

      if (safe_cut_height != milling_points.back().y()) {
        auto point = heightMap.indexToPos(global_index);
        milling_points.emplace_back(point.x(), safe_cut_height, point.z());
      }
    }

    auto real_x = left_to_right ? heightMap.divisions().x_ - 1 : 0;
    auto pos = heightMap.indexToPos(heightMap.globalIndex(real_x, real_z));
    auto next_pos = heightMap.indexToPos(heightMap.globalIndex(
        real_x, (z + 1) * dz * heightMap.pixelCmRatio().first));

    milling_points.emplace_back(pos.x(), min_height, pos.z());
    milling_points.emplace_back(pos.x(), min_height, next_pos.z());
  }

  return milling_points;
}

void PathsGenerator::run() {
  auto roughing_path = roughingPath();
  gCodeSerializer_.serializePath(roughing_path, "1.k16");
}