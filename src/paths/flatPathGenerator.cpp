#include "flatPathGenerator.hpp"
#include "cutter.hpp"
#include "millingPath.hpp"
#include <queue>
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
  auto milling_points = findCutterPositionsFromBoundary(boundary_indices);
  return MillingPath(std::move(milling_points), cutter);
};

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
    if (is_floor(index)) {
      boundary_indices.push_back(index);
      continue;
    }

    /// here check wheter we dont need additional check
    for (int d : d_index) {
      const auto current_index = index + d;

      visited[current_index] = true;
      queue.push(current_index);
    }
  }

  return boundary_indices;
}

std::vector<algebra::Vec3f> FlatPathGenerator::findCutterPositionsFromBoundary(
    const std::vector<uint32_t> &boundaryIndices) const {
  /// here we may need normal map since we have border positions
  /// but we need to offset in direction from normal vector by radius of cutter
}