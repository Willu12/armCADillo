#include "flatPathGenerator.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "segment.hpp"
#include "vec.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <print>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static constexpr float kFloorheight = 1.5f;
static constexpr float kSafeHeight = 5.f;
static constexpr float kEpsilon = 1e-3;
static constexpr float kMillingPrecision = 0.01f;
static constexpr int kMaxIndex = std::numeric_limits<int>::max();

void FlatPathGenerator::setCutter(const Cutter *cutter) { cutter_ = cutter; }
void FlatPathGenerator::setHeightMap(HeightMap *heightMap) {
  heightMap_ = heightMap;
}

MillingPath FlatPathGenerator::generate() {

  auto boundary_indices = findBoundaryIndices();
  // paintBorderRed(boundary_indices);

  auto countour_points = findCutterPositionsFromBoundary(boundary_indices);
  countour_points = removeSelfIntersections(countour_points);
  auto segments = generateSegments(countour_points);
  paintBorderRed(countour_points);
  auto local_paths = generatePaths(segments);

  return combineLocalPaths(local_paths);
};

std::vector<uint32_t> FlatPathGenerator::findBoundaryIndices() {
  std::vector<bool> is_border(heightMap_->data_.size(), false);
  size_t border_size = 0;
  std::vector<uint32_t> border;
  auto is_floor = [&](size_t x, size_t z) {
    return heightMap_->at(heightMap_->globalIndex(x, z)) < kFloorheight + 1e-5f;
  };

  for (size_t x = 0; x < heightMap_->divisions().x_; ++x) {
    for (size_t z = 0; z < heightMap_->divisions().z_; ++z) {

      if (is_floor(x, z)) {
        continue;
      }

      if (x + 1 < heightMap_->divisions().x_ && is_floor(x + 1, z)) {
        const auto global_index = heightMap_->globalIndex(x + 1, z);
        is_border[global_index] = true;
        border_size++;
        border.push_back(global_index);
        boundaryNormalMap_.insert(
            {global_index,
             heightMap_->normalAtIndex(heightMap_->globalIndex(x, z))});
      }
      if (x > 0 && is_floor(x - 1, z)) {
        const auto global_index = heightMap_->globalIndex(x - 1, z);
        is_border[global_index] = true;
        border_size++;
        border.push_back(global_index);

        boundaryNormalMap_.insert(
            {global_index,
             heightMap_->normalAtIndex(heightMap_->globalIndex(x, z))});
      }
      if (z + 1 < heightMap_->divisions().z_ && is_floor(x, z + 1)) {
        const auto global_index = heightMap_->globalIndex(x, z + 1);
        is_border[global_index] = true;
        border_size++;
        border.push_back(global_index);

        boundaryNormalMap_.insert(
            {global_index,
             heightMap_->normalAtIndex(heightMap_->globalIndex(x, z))});
      }
      if (z > 0 && is_floor(x, z - 1)) {
        const auto global_index = heightMap_->globalIndex(x, z - 1);
        is_border[global_index] = true;
        border_size++;
        border.push_back(global_index);

        boundaryNormalMap_.insert(
            {global_index,
             heightMap_->normalAtIndex(heightMap_->globalIndex(x, z))});
      }
    }
  }

  // return border;

  //// find CW boundary path
  auto starting_index = std::ranges::find(is_border, true) - is_border.begin();
  auto index = heightMap_->indexFromGlobalIndex(starting_index);
  std::vector<uint32_t> border_indices;
  std::unordered_set<uint32_t> visited;
  std::unordered_set<uint32_t> blocked;
  border_indices.reserve(border_size);
  auto x = index.first;
  auto z = index.second;

  while (border_indices.size() != border_size) {
    const auto global_index = heightMap_->globalIndex(x, z);
    border_indices.push_back(global_index);

    bool found = false;
    for (uint32_t xx = 0; xx < 3; ++xx) {
      bool break_loop = false;
      for (uint32_t zz = 0; zz < 3; ++zz) {
        if (xx == 1 && zz == 1) {
          continue;
        }
        auto current_x = x + xx - 1;
        auto current_z = z + zz - 1;
        if (current_x >= heightMap_->divisions().x_ ||
            current_z >= heightMap_->divisions().z_) {
          continue;
        }
        const auto current_global_index =
            heightMap_->globalIndex(current_x, current_z);

        if (blocked.contains(current_global_index)) {
          continue;
        }

        if (is_border[current_global_index] &&
            !visited.contains(current_global_index)) {

          visited.insert(current_global_index);
          x = current_x;
          z = current_z;
          break_loop = true;
          found = true;
          break;
        }
      }
      if (break_loop) {
        break;
      }
    }

    if (!found) {
      if (blocked.contains(heightMap_->globalIndex(x, z))) {
        std::ranges::reverse(border_indices);
        return border_indices;
      }
      blocked.insert(heightMap_->globalIndex(x, z));
      border_indices.pop_back();
      auto prev_index = heightMap_->indexFromGlobalIndex(border_indices.back());
      x = prev_index.first;
      z = prev_index.second;
    }
  }
  // return border;
  // std::ranges::reverse(border_indices);
  return border_indices;
}

std::vector<algebra::Vec3f> FlatPathGenerator::findCutterPositionsFromBoundary(
    const std::vector<uint32_t> &boundaryIndices) const {
  /// here we may need normal map since we have border positions
  /// but we need to offset in direction from normal vector by radius of
  /// cutter

  std::vector<algebra::Vec3f> milling_points(boundaryIndices.size());
  for (const auto &[i, index] : boundaryIndices | std::views::enumerate) {
    const auto boundary_pos = heightMap_->indexToPos(index);
    const auto normal = boundaryNormalMap_.at(index);
    const auto flat_normal =
        algebra::Vec3f{normal.x(), 0.f, normal.z()}.normalize();

    const auto cut_point =
        boundary_pos + flat_normal * cutter_->diameter_ / 2.f;
    milling_points[i] = cut_point;
  }

  return milling_points;
}

void FlatPathGenerator::paintBorderRed(
    const std::vector<uint32_t> &boundaryIndices) const {

  for (const auto &index : boundaryIndices) {
    heightMap_->textureData_[4 * index] = 255;
    heightMap_->textureData_[4 * index + 1] = 0;
    heightMap_->textureData_[4 * index + 2] = 0;
  }
  auto index = boundaryIndices.back();
  heightMap_->textureData_[4 * index] = 0;
  heightMap_->textureData_[4 * index + 1] = 0;
  heightMap_->textureData_[4 * index + 2] = 255;
  ////dasdas
  index = boundaryIndices.front();

  heightMap_->textureData_[4 * index] = 0;
  heightMap_->textureData_[4 * index + 1] = 255;
  heightMap_->textureData_[4 * index + 2] = 0;
  heightMap_->texture_->fill(heightMap_->textureData_);
}

void FlatPathGenerator::paintBorderRed(
    const std::vector<algebra::Vec3f> &contour) const {

  for (const auto &point : contour) {
    auto index = heightMap_->posToIndex(point);

    heightMap_->textureData_[4 * index] = 255;
    heightMap_->textureData_[4 * index + 1] = 0;
    heightMap_->textureData_[4 * index + 2] = 0;
  }
  /// last to blue
  auto index = heightMap_->posToIndex(contour.back());
  heightMap_->textureData_[4 * index] = 0;
  heightMap_->textureData_[4 * index + 1] = 0;
  heightMap_->textureData_[4 * index + 2] = 255;
  ////dasdas
  index = heightMap_->posToIndex(contour.front());

  heightMap_->textureData_[4 * index] = 0;
  heightMap_->textureData_[4 * index + 1] = 255;
  heightMap_->textureData_[4 * index + 2] = 0;
  heightMap_->texture_->fill(heightMap_->textureData_);
}

std::vector<std::list<Segment>> FlatPathGenerator::generateSegments(
    const std::vector<algebra::Vec3f> &countourPoints) const {
  ///
  float line_distance = cutter_->diameter_ - kEpsilon;

  const auto &block = heightMap_->block();

  const float half_x = block.dimensions_.x_ / 2.f;
  const float half_z = block.dimensions_.z_ / 2.f;

  const auto starting_point = algebra::Vec3f(
      -half_x - line_distance, block.dimensions_.y_ + line_distance,
      -half_z - line_distance);

  /// process one line with some step

  auto intersects =
      [](const algebra::Vec2f &p0, const algebra::Vec2f &p1,
         const algebra::Vec2f &q0,
         const algebra::Vec2f &q1) -> std::optional<algebra::Vec2f> {
    const auto p01 = p1 - p0;
    const auto q01 = q1 - q0;
    const auto c0 = (q0 - p1).cross2D(p01);
    const auto c1 = (q1 - p1).cross2D(p01);
    const auto c2 = (p0 - q1).cross2D(q01);
    const auto c3 = (p1 - q1).cross2D(q01);
    if (c0 * c1 < 0.f && c2 * c3 < 0.f) {
      const auto w = (p01).cross2D(-1.f * q01);
      const auto w_u = (q0 - p0).cross2D(-1.f * q01);
      return p0 + w_u / w * p01;
    }
    return std::nullopt;
  };

  std::vector<std::list<Segment>> segments;
  const auto min_z = -half_z - line_distance;
  const auto max_z = half_z + line_distance;
  const auto min_x = -half_x - line_distance;
  const auto max_x = -min_x;

  //// process each line and check all possible cuts with countour
  auto line_count = static_cast<uint32_t>((max_z - min_z) / line_distance);
  for (uint32_t i = 0; i < line_count; ++i) {
    std::list<Segment> line_segments;
    auto curr_z = min_z + static_cast<float>(i) * line_distance;

    algebra::Vec2f p0 = {min_x, curr_z};
    algebra::Vec2f p1 = {max_x, curr_z};

    std::vector<algebra::Vec2f> line_intersections;

    for (int point_index = 0; point_index < countourPoints.size();
         ++point_index) {
      const auto point = countourPoints[point_index];
      const auto next_point =
          countourPoints[(point_index + 1) % countourPoints.size()];

      const auto q0 = algebra::Vec2f{point.x(), point.z()};
      const auto q1 = algebra::Vec2f{next_point.x(), next_point.z()};

      auto intersection = intersects(p0, p1, q0, q1);
      if (intersection) {
        line_intersections.push_back(*intersection);
      }
    }

    std::ranges::sort(line_intersections,
                      [](const algebra::Vec2f &p, const algebra::Vec2f &q) {
                        return p.x() < q.x();
                      });

    auto prev = p0;
    bool inside = false;
    for (const auto &intersection : line_intersections) {
      Segment segment{.start_ = prev, .end_ = intersection};
      if (!inside) {
        line_segments.push_back(segment);
      }
      inside = !inside;
      prev = intersection;
    }

    line_segments.emplace_back(prev, p1);

    segments.push_back(line_segments);
  }

  return segments;
}

std::vector<std::vector<algebra::Vec3f>> FlatPathGenerator::generatePaths(
    std::vector<std::list<Segment>> &segments) const {
  /// we have vector of segments

  std::vector<std::vector<algebra::Vec3f>> paths;

  while (true) {
    auto first_line_index = kMaxIndex;

    for (const auto &[index, segment] : segments | std::views::enumerate) {
      if (segment.size() > 0) {
        first_line_index = static_cast<int>(index);
        break;
      }
    }

    /// all segments are cleared
    if (first_line_index >= kMaxIndex) {
      break;
    }

    /// process paths
    std::vector<algebra::Vec3f> path;
    auto &first_line = segments[first_line_index];
    auto previous_segment = segments[first_line_index].begin();

    const auto start_point =
        algebra::Vec3f(previous_segment->start_.x(), kFloorheight,
                       previous_segment->start_.y());
    const auto second_point = algebra::Vec3f(
        previous_segment->end_.x(), kFloorheight, previous_segment->end_.y());

    path.push_back(start_point);
    path.push_back(second_point);

    auto previous_line_index = first_line_index;
    first_line.erase(previous_segment);

    for (int line_index = first_line_index + 1; line_index < segments.size();
         ++line_index) {
      auto &line_segments = segments[line_index];
      /// 1. there are no more segments in this line
      if (line_segments.empty()) {
        continue;
      }

      bool reversed = ((line_index - first_line_index) % 2) != 0;

      auto best_segment = line_segments.begin();
      float min_dist = std::numeric_limits<float>::max();

      for (auto segment_it = segments[line_index].begin();
           segment_it != segments[line_index].end(); ++segment_it) {
        auto dist =
            reversed ? (previous_segment->end_ - segment_it->end_).length()
                     : (previous_segment->start_ - segment_it->start_).length();
        if (dist < min_dist) {
          min_dist = dist;
          best_segment = segment_it;
        }
      }

      // Zig Zag order
      if (reversed) {
        path.emplace_back(best_segment->end_.x(), kFloorheight,
                          best_segment->end_.y());
        path.emplace_back(best_segment->start_.x(), kFloorheight,
                          best_segment->start_.y());
      } else {
        path.emplace_back(best_segment->start_.x(), kFloorheight,
                          best_segment->start_.y());
        path.emplace_back(best_segment->end_.x(), kFloorheight,
                          best_segment->end_.y());
      }

      /// remove previous segment
      line_segments.erase(best_segment);
      previous_segment = best_segment;
      // previous_line_index = line_index;
    }

    paths.push_back(path);
  }
  return paths;
}

std::vector<algebra::Vec3f> FlatPathGenerator::removeSelfIntersections(
    const std::vector<algebra::Vec3f> &points) const {

  /*
const auto block = heightMap_->block();
const auto diag = algebra::Vec2f{0.F, block.dimensions_.x_};
const auto p = algebra::Vec2f{0.F, block.dimensions_.x_ / 2.F};
size_t outer_ind = kMaxIndex;
for (auto i = 0U; i < points.size() - 1; ++i) {
auto p0 = algebra::Vec2f{points[i].x(), points[i].z()};
auto p1 = algebra::Vec2f{points[i + 1].x(), points[i + 1].z()};
if ((p0 - p).cross2D(diag) * (p1 - p).cross2D(diag) > 0.F) {
  continue;
}
if (outer_ind == kMaxIndex || points[outer_ind].z() < points[i].z()) {
  outer_ind = i;
}
}
*/

  auto start_index = 500;
  auto ind = start_index;
  auto contour_points = std::vector<algebra::Vec3f>();
  // contour_points.reserve(border_count);
  contour_points.push_back(points[ind]);
  for (auto steps = 0U; steps < points.size(); ++steps) {
    auto next_ind = (ind + 1) % points.size();

    auto p0 = algebra::Vec2f{points[ind].x(), points[ind].z()};
    auto p1 = algebra::Vec2f{points[next_ind].x(), points[next_ind].z()};
    auto dirp = p1 - p0;

    auto old_ind = ind;
    ind = next_ind;
    for (auto j = next_ind; j != old_ind; j = (j + 1) % points.size()) {
      auto next_j = (j + 1) % points.size();

      auto q0 = algebra::Vec2f{points[j].x(), points[j].z()};
      auto q1 = algebra::Vec2f{points[next_j].x(), points[next_j].z()};
      auto dirq = q1 - q0;

      auto c1 = (q0 - p1).cross2D(dirp);
      auto c2 = (q1 - p1).cross2D(dirp);
      auto c3 = (p0 - q1).cross2D(dirq);
      auto c4 = (p1 - q1).cross2D(dirq);

      if (c1 * c2 < 0.F && c3 * c4 < 0.F) {
        if (c1 > c2) {
          ind = j;
        } else {
          ind = j + 1;
        }

        break;
      }
    }

    contour_points.push_back(points[ind]);
    if (ind == start_index) {
      break;
    }
  }
  return contour_points;
}

MillingPath FlatPathGenerator::combineLocalPaths(
    const std::vector<std::vector<algebra::Vec3f>> &localPaths) const {
  std::vector<algebra::Vec3f> global_path;

  for (const auto &path : localPaths) {
    auto first_point = path.front();
    auto last_point = path.back();

    global_path.emplace_back(first_point.x(), kSafeHeight, first_point.z());
    global_path.insert(global_path.end(), path.begin(), path.end());
    global_path.emplace_back(last_point.x(), kSafeHeight, last_point.z());
  }

  return MillingPath(global_path, *cutter_);
}