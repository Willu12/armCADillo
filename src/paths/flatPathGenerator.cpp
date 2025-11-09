#include "flatPathGenerator.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "vec.hpp"
#include <queue>
#include <ranges>
#include <vector>

static constexpr float kFloorheight = 1.5f;
static constexpr uint32_t kStartingIndex = 0;

MillingPath FlatPathGenerator::generate(HeightMap &heightMap) const {
  Cutter cutter{
      .type_ = Cutter::Type::Flat,
      .diameter_ = 1.0f,
      .height_ = 1.0f,
  };

  auto boundary_indices = findBoundaryIndices(heightMap);
  paintBorderRed(heightMap, boundary_indices);

  auto milling_points =
      findCutterPositionsFromBoundary(heightMap, boundary_indices, cutter);
  return MillingPath(std::move(milling_points), cutter);
};

namespace {
bool checkBounds(const HeightMap &heightMap, uint32_t index, uint32_t dx,
                 uint32_t dz) {
  const auto index_x = index % heightMap.divisions().x_;
  const auto index_z = index / heightMap.divisions().x_;

  return index_x - dx >= 0 && index_x + dx < heightMap.divisions().x_ &&
         index_z - dz >= 0 && index_z + dz < heightMap.divisions().z_;
}
} // namespace

std::vector<uint32_t>
FlatPathGenerator::findBoundaryIndices(HeightMap &heightMap) const {
  /// find boundary points using flood fill
  std::vector<uint32_t> boundary_indices;

  std::vector<bool> visited(heightMap.divisions().x_ * heightMap.divisions().z_,
                            false);

  const auto is_floor = [&](uint32_t index) {
    auto position = heightMap.indexToPos(index);
    return position.y() <= kFloorheight;
  };

  std::queue<uint32_t> queue;

  /// init with (0,0)
  queue.emplace(kStartingIndex);
  visited[kStartingIndex] = true;

  const auto d_x = {1, -1, 0, 0};
  const auto d_z = {0, 0, 1, -1};

  while (!queue.empty()) {
    auto index = queue.front();
    queue.pop();

    /// check if is border
    if (!is_floor(index)) {
      boundary_indices.push_back(index);
      continue;
    }

    /// here check wheter we dont need additional check
    for (int dx : d_x) {
      for (int dz : d_z) {

        if (!checkBounds(heightMap, index, dx, dz)) {
          continue;
        }

        auto d = dx + dz * heightMap.divisions_.x_;
        const auto neighbour_index = index + d;
        if (!visited[neighbour_index]) {
          visited[neighbour_index] = true;
          queue.push(neighbour_index);
        }
      }
    }
  }
  return boundary_indices;
}

std::vector<algebra::Vec3f> FlatPathGenerator::findCutterPositionsFromBoundary(
    const HeightMap &heightMap, const std::vector<uint32_t> &boundaryIndices,
    const Cutter &cutter) const {
  /// here we may need normal map since we have border positions
  /// but we need to offset in direction from normal vector by radius of cutter

  std::vector<algebra::Vec3f> milling_points(boundaryIndices.size());
  for (const auto &[i, index] : boundaryIndices | std::views::enumerate) {
    const auto boundary_pos = heightMap.indexToPos(index);
    const auto normal = heightMap.normalAtIndex(index);
    const auto flat_normal =
        algebra::Vec3f{normal.x(), 0.f, normal.z()}.normalize();

    const auto cut_point = boundary_pos + flat_normal * cutter.diameter_ / 2.f;
    // cut_point.y() = kFloorheight;
    milling_points[i] = cut_point;
  }

  return milling_points;
}

void FlatPathGenerator::paintBorderRed(
    HeightMap &heightMap, const std::vector<uint32_t> &boundaryIndices) const {
  for (const auto &index : boundaryIndices) {
    heightMap.textureData_[4 * index] = 255;
    heightMap.textureData_[4 * index + 1] = 0;
    heightMap.textureData_[4 * index + 2] = 0;
  }
  heightMap.texture_->fill(heightMap.textureData_);
}