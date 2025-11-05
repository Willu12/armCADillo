#include "heightMap.hpp"
#include "block.hpp"
#include "cutter.hpp"
#include "vec.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>

float HeightMap::findMinimumSafeHeightForCut(uint32_t index,
                                             const Cutter &cutter) const {
  if (cutter.type_ == Cutter::Type::Flat) {
    throw std::runtime_error("Cutter for roughing should never be flat!");
  }

  auto center_position = indexToPos(index);
  auto x_index = index % divisions_.x_;
  auto z_index = index / divisions_.x_;

  auto cutter_radius_px = static_cast<int32_t>(
      (cutter.diameter_ / 2.f) * static_cast<float>(pixelCmRatio().first));

  auto max_height = std::numeric_limits<float>::lowest();

  /// we create bounding box
  for (int32_t x_cut = -cutter_radius_px; x_cut < cutter_radius_px; ++x_cut) {
    for (int32_t z_cut = -cutter_radius_px; z_cut < cutter_radius_px; ++z_cut) {
      if (x_index + x_cut < 0 || z_cut + z_index < 0 ||
          x_index + x_cut >= divisions_.x_ ||
          z_index + z_cut >= divisions_.z_) {
        continue;
      }

      auto position = indexToPos(globalIndex(x_index + x_cut, z_index + z_cut));
      float x_diff_sq = std::pow(position.x() - center_position.x(), 2.f);
      float z_diff_sq = std::pow(position.z() - center_position.z(), 2.f);
      float r_squared = cutter.diameter_ * cutter.diameter_ / 4.f;

      /// check if its inside the circle
      if (x_diff_sq + z_diff_sq > r_squared) {
        continue;
      }

      float cut_height = center_position.y() - cutter.diameter_ / 2.f +
                         std::sqrt(r_squared - x_diff_sq - z_diff_sq);
      max_height = std::max(cut_height, max_height);
    }
  }
  return max_height;
}

std::pair<uint32_t, uint32_t> HeightMap::pixelCmRatio() const {
  auto x_ratio = static_cast<float>(divisions_.x_) / block_->dimensions_.x_;
  auto z_ratio = static_cast<float>(divisions_.z_) / block_->dimensions_.z_;

  return {x_ratio, z_ratio};
}

////////////////////////////////////////////////////////////
//// height map looks like
/// Z|
///  |
///  |
///  |
/// 0*---------
///  0       X

algebra::Vec3f HeightMap::indexToPos(uint32_t index) const {
  const auto x_index = index % divisions_.x_;
  const auto z_index = index / divisions_.x_;

  auto world_position = [](float length, uint32_t index, uint32_t maxIndex) {
    return -length / 2.f +
           length * static_cast<float>(index) / static_cast<float>(maxIndex);
  };

  const auto x = world_position(block_->dimensions_.x_, x_index, divisions_.x_);
  const auto z = world_position(block_->dimensions_.z_, z_index, divisions_.z_);
  const auto y = data_[index];

  return algebra::Vec3f(x, y, z);
}

uint32_t HeightMap::posToIndex(const algebra::Vec3f &position) const {

  auto get_index = [&](float x, float len, uint32_t divisions) {
    return static_cast<int>(static_cast<float>(divisions) *
                            ((x + len / 2.f) / len));
  };

  int x_index = get_index(position.x(), block_->dimensions_.x_, divisions_.x_);
  int z_index = get_index(position.z(), block_->dimensions_.z_, divisions_.z_);

  return z_index * divisions_.x_ + x_index;
}

uint32_t HeightMap::globalIndex(uint32_t x, uint32_t z) const {
  return z * divisions_.x_ + x;
}

void HeightMap::updateTexture() {
  std::vector<uint8_t> texture_data(data_.size());

  const auto min_height = 0.f;
  const auto max_height = 5.f;

  for (int i = 0; i < texture_data.size(); ++i) {
    texture_data[i] = static_cast<uint8_t>((data_[i] / max_height) * 255.f);
  }

  texture_->fill(texture_data);
}

algebra::Vec3f &HeightMap::normalAtIndex(uint32_t index) {
  return normalData_[index];
}
const algebra::Vec3f &HeightMap::normalAtIndex(uint32_t index) const {
  return normalData_[index];
}