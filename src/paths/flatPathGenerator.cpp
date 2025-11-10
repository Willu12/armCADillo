#include "flatPathGenerator.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "segment.hpp"
#include "vec.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <vector>

static constexpr float kFloorheight = 1.5f;
static constexpr float kEpsilon = 1e-3;
static constexpr float kMillingPrecision = 0.0001f;
static constexpr int kMaxIndex = std::numeric_limits<int>::max();

void FlatPathGenerator::setCutter(const Cutter *cutter) { cutter_ = cutter; }
void FlatPathGenerator::setHeightMap(HeightMap *heightMap) {
  heightMap_ = heightMap;
}

MillingPath FlatPathGenerator::generate() const {

  auto boundary_normal_map = createBoundaryNormalMap();
  auto boundary_indices = findBoundaryIndices();
  // paintBorderRed(boundary_indices);

  auto countour_points =
      findCutterPositionsFromBoundary(boundary_indices, boundary_normal_map);
  // countour_points = removeSelfIntersections(countour_points);
  paintBorderRed(countour_points);
  MillingPath path({}, *cutter_);
  return path;
  // auto segments = generateSegments(countour_points);
  // auto milling_paths = generatePaths(segments);
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

std::vector<uint32_t> FlatPathGenerator::findBoundaryIndices() const {
  std::vector<uint32_t> boundary_indices;
  auto &divisions = heightMap_->divisions_;
  uint32_t start = 0;

  const auto is_floor = [&](uint32_t index) {
    auto position = heightMap_->indexToPos(index);
    return position.y() <= kFloorheight;
  };

  while (start < divisions.x_ * divisions.z_ && is_floor(start)) {
    ++start;
  }

  /// get them in correct order
  int dir = 0;
  auto current = start;
  while (true) {

    boundary_indices.push_back(current);
    bool found_next = false;

    for (int turn = -1; turn <= 2 && !found_next; ++turn) {
      int ndir = (dir + turn + 4) % 4;
      int dx = (ndir == 0) - (ndir == 2);
      int dz = (ndir == 1) - (ndir == 3);

      if (!checkBounds(*heightMap_, current, dx, dz)) {
        continue;
      }

      auto next = current + dx + dz * heightMap_->divisions().x_;
      if (!is_floor(next)) {
        current = next;
        dir = ndir;
        found_next = true;
      }
    }

    if (!found_next || current == start) {
      break;
    }
  }
  std::ranges::reverse(boundary_indices);
  return boundary_indices;
}

std::vector<algebra::Vec3f> FlatPathGenerator::findCutterPositionsFromBoundary(
    const std::vector<uint32_t> &boundaryIndices,
    const std::unordered_map<uint32_t, algebra::Vec3f> &boundaryNormalMap)
    const {
  /// here we may need normal map since we have border positions
  /// but we need to offset in direction from normal vector by radius of
  /// cutter

  std::vector<algebra::Vec3f> milling_points(boundaryIndices.size());
  for (const auto &[i, index] : boundaryIndices | std::views::enumerate) {
    const auto boundary_pos = heightMap_->indexToPos(index);
    const auto normal = boundaryNormalMap.at(index);
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

  auto intersects_contour = [&countourPoints](const algebra::Vec2f &point) {
    auto it = std::ranges::find_if(
        countourPoints, [&point](const algebra::Vec3f &contourPoint) {
          algebra::Vec2f countour_point_xz{contourPoint.x(), contourPoint.z()};
          return (countour_point_xz - point).length() < kEpsilon;
        });
    return it != countourPoints.end();
  };
  /// precision is one mm - this may be super slow

  const auto min_z = -half_z - line_distance;
  const auto max_z = -min_z;
  const auto min_x = -half_x - line_distance;
  const auto max_x = -min_x;

  bool inside_border = false;
  std::vector<std::list<Segment>> segments;

  /// these could be changed to go along the contour
  for (float zz = min_z; zz < max_z; zz += line_distance) {
    float last_x = min_x;
    std::list<Segment> current_line_segments;
    for (float xx = min_x; xx < max_x; xx += kMillingPrecision) {
      const algebra::Vec2f p(xx, zz);

      if (intersects_contour(p)) {
        if (!inside_border) {
          current_line_segments.push_back(
              {.start_ = {last_x, zz}, .end_ = {xx, zz}});
          // last_x = xx;
          inside_border = true;
        } else {
          inside_border = false;
          last_x = xx;
        }
      }
    }
    // add final segment
    current_line_segments.push_back(
        {.start_ = {last_x, zz}, .end_ = {max_x, zz}});

    segments.push_back(current_line_segments);
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
    if (first_line_index == kMaxIndex) {
      break;
    }

    /// process paths

    std::vector<algebra::Vec3f> path;
    const auto &first_segment = segments[first_line_index].front();
    const auto start_point = algebra::Vec3f(
        first_segment.start_.x(), kFloorheight, first_segment.start_.y());
    const auto second_point = algebra::Vec3f(
        first_segment.end_.x(), kFloorheight, first_segment.end_.y());

    auto previous_segment = segments[first_line_index].begin();
    auto previous_line_index = first_line_index;
    path.push_back(start_point);
    path.push_back(second_point);

    for (int line_index = first_line_index + 1; line_index < segments.size();
         line_index++) {

      /// 1. there are no more segments in this line
      if (segments[line_index].size() == 0) {
        continue;
      }

      bool reversed = (line_index - first_line_index) % 2 != 0;

      auto curr_segment = segments[line_index].begin();
      float min_dist = std::numeric_limits<float>::max();
      for (auto segment_it = segments[line_index].begin();
           segment_it != segments[line_index].end(); ++segment_it) {

        auto dist =
            reversed ? (previous_segment->end_ - segment_it->end_).length()
                     : (previous_segment->start_ - segment_it->start_).length();
        if (dist < min_dist) {
          min_dist = dist;
          curr_segment = segment_it;
        }
      }

      if (reversed) {
        path.emplace_back(curr_segment->end_.x(), kFloorheight,
                          curr_segment->end_.y());
        path.emplace_back(curr_segment->start_.x(), kFloorheight,
                          curr_segment->start_.y());
      } else {
        path.emplace_back(curr_segment->start_.x(), kFloorheight,
                          curr_segment->start_.y());
        path.emplace_back(curr_segment->end_.x(), kFloorheight,
                          curr_segment->end_.y());
      }

      /// remove previous segment
      segments[previous_line_index].erase(previous_segment);
      previous_segment = curr_segment;
      previous_line_index = line_index;
    }

    paths.push_back(path);
  }
  return paths;
}
std::vector<algebra::Vec3f> FlatPathGenerator::removeSelfIntersections(
    const std::vector<algebra::Vec3f> &points) const {
  {
    auto contour_points = std::vector<algebra::Vec3f>();
    // contour_points.reserve(border_count);

    if (points.size() < 4) {
      return points;
    }

    auto intersects = [](const algebra::Vec2f &p1, const algebra::Vec2f &p2,
                         const algebra::Vec2f &q1, const algebra::Vec2f &q2) {
      auto c1 = (q1 - p1).cross2D(p2 - p1);
      auto c2 = (q2 - p1).cross2D(p2 - p1);
      auto c3 = (p1 - q1).cross2D(q2 - q1);
      auto c4 = (p2 - p1).cross2D(q2 - q1);
      return c1 * c2 < 0 && c3 * c4 < 0;
    };

    auto ind = 0;
    contour_points.push_back(points[ind]);

    for (uint32_t steps = 0; steps < points.size(); ++steps) {
      uint32_t next_ind = (ind + 1) % points.size();
      algebra::Vec2f p0(points[ind].x(), points[ind].z());
      algebra::Vec2f p1(points[next_ind].x(), points[next_ind].z());

      bool found_intersection = false;

      for (uint32_t j = 0; j < points.size() - 1; ++j) {
        uint32_t next_j = (j + 1) % points.size();
        if (j == ind || next_j == ind || j == next_ind || next_j == next_ind) {
          continue; // skip adjacent or identical segments
        }

        algebra::Vec2f q0(points[j].x(), points[j].z());
        algebra::Vec2f q1(points[next_j].x(), points[next_j].z());

        if (intersects(p0, p1, q0, q1)) {
          // jump to intersection region
          ind = next_j;
          found_intersection = true;
          break;
        }
      }

      contour_points.push_back(points[next_ind]);

      if (found_intersection) {
        ind = (ind + 1) % points.size();
        contour_points.push_back(points[ind]);
      } else {
        ind = next_ind;
      }

      if (ind == 0) {
        break;
      }
    }

    return contour_points;
  }
}

std::unordered_map<uint32_t, algebra::Vec3f>
FlatPathGenerator::createBoundaryNormalMap() const {
  /// find boundary points using flood fill
  std::unordered_map<uint32_t, algebra::Vec3f> boundary_normal_map;

  std::vector<bool> visited(
      heightMap_->divisions().x_ * heightMap_->divisions().z_, false);

  const auto is_floor = [&](uint32_t index) {
    auto position = heightMap_->indexToPos(index);
    return position.y() <= kFloorheight;
  };

  std::queue<uint32_t> queue;

  /// We start from the middle
  const uint32_t starting_index =
      heightMap_->divisions_.x_ * heightMap_->divisions().z_ / 2;

  queue.emplace(starting_index);
  visited[starting_index] = true;

  const auto d_x = {1, -1, 0, 0};
  const auto d_z = {0, 0, 1, -1};

  while (!queue.empty()) {
    auto index = queue.front();
    queue.pop();

    for (int dx : d_x) {
      for (int dz : d_z) {

        if (!checkBounds(*heightMap_, index, dx, dz)) {
          continue;
        }

        auto d = dx + dz * heightMap_->divisions_.x_;
        const auto neighbour_index = index + d;
        if (!visited[neighbour_index]) {
          visited[neighbour_index] = true;

          /// check if is border
          if (is_floor(neighbour_index)) {
            boundary_normal_map.insert(
                {neighbour_index, heightMap_->normalAtIndex(index)});
            continue;
          }

          queue.push(neighbour_index);
        }
      }
    }
  }
  return boundary_normal_map;
}
