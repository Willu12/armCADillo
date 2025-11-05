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

MillingPath FlatPathGenerator::generate(const HeightMap &heightMap) const {
  Cutter cutter{
      .type_ = Cutter::Type::Flat,
      .diameter_ = 1.0f,
      .height_ = 1.0f,
  };

  auto boundary_indices = findBoundaryIndices(heightMap);
  auto milling_points =
      findCutterPositionsFromBoundary(heightMap, boundary_indices, cutter);
  return MillingPath(std::move(milling_points), cutter);
};

namespace {
bool checkBounds(const HeightMap &heightMap, uint32_t index, uint32_t d) {
  const auto index_x = index % heightMap.divisions().x_;
  const auto index_z = index / heightMap.divisions().x_;

  const auto d_x = d % heightMap.divisions().x_;
  const auto d_z = d / heightMap.divisions().z_;

  return index_x - d_x >= 0 && index_x + d_x < heightMap.divisions().x_ &&
         index_z - d_z >= 0 && index_z + d_z < heightMap.divisions().z_;
}
} // namespace

std::vector<uint32_t>
FlatPathGenerator::findBoundaryIndices(const HeightMap &heightMap) const {
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

  const int d_index[4] = {-1, 1, static_cast<int>(heightMap.divisions_.x_),
                          -static_cast<int>(heightMap.divisions().x_)};

  while (!queue.empty()) {
    auto index = queue.front();
    queue.pop();

    /// check if is border
    if (!is_floor(index)) {
      boundary_indices.push_back(index);
      continue;
    }

    /// here check wheter we dont need additional check
    for (int d : d_index) {
      if (!checkBounds(heightMap, index, d)) {
        continue;
      }

      const auto neighbour_index = index + d;
      if (!visited[neighbour_index]) {
        visited[neighbour_index] = true;
        queue.push(neighbour_index);
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
    auto boundary_pos = heightMap.indexToPos(index);
    auto normal = heightMap.normalAtIndex(index);
    auto cut_point = boundary_pos; //+ normal * cutter.diameter_ / 2.f;
    cut_point.y() = kFloorheight;
    milling_points[i] = cut_point;
  }

  return milling_points;
}