#include "detailedPathGenerator.hpp"
#include "bezierSurface.hpp"
#include "intersectionTexture.hpp"
#include "millingPath.hpp"
#include "normalOffsetSurface.hpp"
#include "plane.hpp"
#include "rdp.hpp"
#include "vec.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <print>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static constexpr float kFloorHeight = 0.f;
static constexpr float kFloorHeightPath = 1.5f;

void DetailedPathGenerator::generate() {
  /// This section assumes that every model surface has proper intersection
  /// texture and all sections that should be trimmed are set.

  for (auto *surface : model_->surfaces()) {
    setFloorAsTrimmed(*surface);

    auto segments = generateLineSegments(*surface);
    auto surface_paths = generateSurfacePaths(*surface, segments);
    auto path = combineSurfacePaths(surface_paths);

    auto milling_path = MillingPath{path, cutter_};
    GCodeSerializer::serializePath(milling_path, surface->getName() + ".k08");
  }
}

void DetailedPathGenerator::setFloorAsTrimmed(
    BezierSurface &intersectableSurface) const {
  auto &intersection_texture = *intersectableSurface.getIntersectionTexture();

  auto size = intersection_texture.getSize();
  auto offset_surface = algebra::NormalOffsetSurface(
      &intersectableSurface.getAlgebraSurfaceC0(), cutter_.radius());

  for (int y = 0; y < size.height; ++y) {
    for (int x = 0; x < size.width; ++x) {
      auto uv = intersection_texture.uv(x, y);
      auto p = offset_surface.value(uv);

      if (p.y() < kFloorHeight + cutter_.radius()) {
        intersection_texture.setCellType(x, y,
                                         IntersectionTexture::CellType::Trim);
      }
    }
  }
  intersection_texture.update();
}

std::vector<std::vector<DetailedPathGenerator::Coord>>
DetailedPathGenerator::generateLineSegments(BezierSurface &surface) {
  std::vector<std::vector<Coord>> lines;
  lines.reserve(lines_);

  const auto &tex = *surface.getIntersectionTexture();
  const auto size = tex.getSize();
  const uint32_t max_index = size.height;
  const uint32_t step = max_index / lines_;

  auto trimmed = [&](uint32_t x, uint32_t y) {
    return tex.getCellType(x, y) != IntersectionTexture::CellType::Keep;
  };

  for (uint32_t line = 0; line < max_index; line += step) {
    std::vector<Coord> segments;

    for (uint32_t i = 0; i < max_index; ++i) {
      uint32_t x = (direction_ == Direction::Horizontal) ? i : line;
      uint32_t y = (direction_ == Direction::Horizontal) ? line : i;

      bool curr_trim = trimmed(x, y);

      if ((i == 0 || i == max_index - 1) && !curr_trim) {
        segments.emplace_back(x, y);
      }

      if (i < max_index - 1) {
        uint32_t x_next = (direction_ == Direction::Horizontal) ? i + 1 : line;
        uint32_t y_next = (direction_ == Direction::Horizontal) ? line : i + 1;

        bool next_trim = trimmed(x_next, y_next);

        if (curr_trim != next_trim) {
          segments.emplace_back(x, y);
        }
      }
    }
    if (segments.size() % 2 != 0) {
      continue;
    }
    lines.emplace_back(std::move(segments));
  }

  return lines;
}

void DetailedPathGenerator::colorSegments(
    BezierSurface &surface, const std::vector<std::vector<Coord>> &segments) {
  auto &intersection_texture = *surface.getIntersectionTexture();

  for (const auto &line : segments) {
    for (const auto &coord : line) {
      intersection_texture.setColor(coord.x, coord.y, Color::Red());
    }
  }
  intersection_texture.update();
}

std::vector<std::vector<algebra::Vec3f>>
DetailedPathGenerator::generateSurfacePaths(
    const BezierSurface &surface,
    std::vector<std::vector<Coord>> &segments) const {
  using Coord = DetailedPathGenerator::Coord;

  std::vector<std::vector<algebra::Vec3f>> paths;
  const algebra::NormalOffsetSurface offset_surface(
      &surface.getAlgebraSurfaceC0(), cutter_.radius());

  const int lines_count = static_cast<int>(segments.size());
  const auto &intersection_texture = surface.getIntersectionTexture();

  auto intersects_texture = [&](Coord coord) {
    return intersects(coord, intersection_texture);
  };
  while (true) {

    int line = -1;
    for (int i = 0; i < lines_count; ++i) {
      if (!segments[i].empty()) {
        line = i;
        break;
      }
    }

    if (line == -1) {
      break;
    }

    std::vector<algebra::Vec3f> path;
    bool reverse = true;

    auto &first_line = segments[line];
    if (first_line.size() < 2) {
      throw std::runtime_error("invalid size of line");
      break;
    }

    Coord s0 = first_line[0];
    Coord s1 = first_line[1];
    first_line.erase(first_line.begin(), first_line.begin() + 2);

    {
      auto pts = generateLinePoints(surface, s0, s1);
      path.insert(path.end(), pts.begin(), pts.end());
    }

    Coord last_segment_start = s0;
    Coord last_segment_end = s1;

    for (int next_line_index = line + 1; next_line_index < lines_count;
         ++next_line_index) {

      auto &next_line = segments[next_line_index];
      if (next_line.size() < 2) {
        // throw std::runtime_error("invalid size of line");
        break;
      }

      int best_index = -1;
      float best_dist = std::numeric_limits<float>::infinity();
      for (auto i = 0; i < next_line.size(); i += 2) {

        if (reverse) {
          auto curr_segment_start = next_line[i];
          auto prev = offset_surface.value(surface.getIntersectionTexture().uv(
              last_segment_start.x, last_segment_start.y));
          auto curr = offset_surface.value(surface.getIntersectionTexture().uv(
              curr_segment_start.x, curr_segment_start.y));

          auto d = (prev - curr).length();
          if (d < best_dist) {
            best_dist = d;
            best_index = i;
          }
        } else {
          auto curr_segment_end = next_line[i + 1];
          auto prev = offset_surface.value(surface.getIntersectionTexture().uv(
              last_segment_end.x, last_segment_end.y));
          auto curr = offset_surface.value(surface.getIntersectionTexture().uv(
              curr_segment_end.x, curr_segment_end.y));

          auto d = (prev - curr).length();
          if (d < best_dist) {
            best_dist = d;
            best_index = i;
          }
        }
      }

      Coord start = next_line[best_index];
      Coord end = next_line[best_index + 1];

      if (reverse) {
        std::swap(start, end);
      }

      auto last_end = path.back();
      auto next_point =
          offset_surface.value(intersection_texture.uv(start.x, start.y));
      next_point.y() += 1.5f - cutter_.radius();
      if ((last_end - next_point).length() > 0.5f) {
        break;
      }

      /// TODO:
      /// Maybe go along the intersection curve till x/y are proper.
      {
        //  auto bridge_points = createSafeBridge(last_segment_end, start,
        //  surface);
        // auto last_point = path.back();
        // auto ext_point =
        offset_surface.value(intersection_texture.uv(start.x, start.y));
        // last_point.y() = last_point.y() + 1.5f - cutter_.radius();
        //  bridge_points.front().y();
        //  ext_point.y() = ext_point.y() + 1.5f - cutter_.radius();
        //  std::vector<algebra::Vec3f> bridge = {last_point, ext_point};
        //  fixIntersectionLine(bridge, surface);
        // bridge_points.front().y();
        //  path.emplace_back(last_point);
        // path.emplace_back(ext_point);
        // path.insert(path.end(), bridge_points.begin(), bridge_points.end());
      }
      {
        /*
         */
      }
      ///
      auto pts = generateLinePoints(surface, start, end);
      path.insert(path.end(), pts.begin(), pts.end());

      next_line.erase(next_line.begin() + best_index,
                      next_line.begin() + best_index + 2);

      // last_point = end;
      last_segment_start = start;
      last_segment_end = end;
      reverse = !reverse;
    }

    paths.push_back(std::move(path));
  }

  return paths;
}

std::vector<algebra::Vec3f>
DetailedPathGenerator::generateLinePoints(const BezierSurface &surface,
                                          Coord start, Coord end) const {
  std::vector<algebra::Vec3f> points;

  const auto &texture = surface.getIntersectionTexture();
  auto offset_surface = algebra::NormalOffsetSurface(
      &surface.getAlgebraSurfaceC0(), cutter_.radius());
  bool reversed = false;
  if (start.x > end.x || start.y > end.y) {
    reversed = true;
    std::swap(start, end);
  }

  Coord delta =
      start.y == end.y ? Coord{.x = 1, .y = 0} : Coord{.x = 0, .y = 1};

  Coord current_coord{.x = start.x, .y = start.y};
  while (current_coord != end) {
    auto uv = texture.uv(current_coord.x, current_coord.y);
    auto offset_point = offset_surface.value(uv);
    offset_point.y() += kFloorHeightPath - cutter_.radius();
    points.emplace_back(offset_point);

    current_coord.x += delta.x;
    current_coord.y += delta.y;
  }

  /// last_point
  auto uv = texture.uv(end.x, end.y);
  auto offset_point = offset_surface.value(uv);
  offset_point.y() += kFloorHeightPath - cutter_.radius();
  points.emplace_back(offset_point);

  if (reversed) {
    std::ranges::reverse(points);
  }

  fixIntersectionLine(points, surface);

  algebra::Plane plane =
      start.x == end.x ? algebra::Plane::YZ : algebra::Plane::XZ;
  return points; // algebra::RDP::reducePoints(points, 10e-5, plane);
}

std::vector<algebra::Vec3f> DetailedPathGenerator::combineSurfacePaths(
    const std::vector<std::vector<algebra::Vec3f>> &surfacePaths) const {
  std::vector<algebra::Vec3f> combined_path;

  for (int i = 0; i < surfacePaths.size(); ++i) {
    const auto &path = surfacePaths.at(i);
    combined_path.insert(combined_path.end(), path.begin(), path.end());

    if (i < surfacePaths.size() - 1) {
      const auto &next_path = surfacePaths.at(i + 1);
      combined_path.emplace_back(path.back().x(), 5.f, path.back().z());
      combined_path.emplace_back(next_path.front().x(), 5.f,
                                 next_path.front().z());
    }
  }

  return combined_path;
}

void DetailedPathGenerator::generatePathForIntersectionCurve(
    const IntersectionCurve &intersectionCurve) const {
  const auto &points = intersectionCurve.getPolyline().getPoints();
  std::vector<algebra::Vec3f> milling_points;

  std::vector<algebra::Vec3f> current_points;

  for (const auto &p : points) {
    auto new_point = p - algebra::Vec3f{0.f, cutter_.radius() - 1.5f, 0.f};
    if (new_point.y() > 1.5) {
      current_points.push_back(new_point);
    } else {
      milling_points.insert(milling_points.begin(), current_points.begin(),
                            current_points.end());
      current_points.clear();
    }
  }

  std::ranges::sort(milling_points,
                    [](const auto &a, const auto &b) { return a.z() > b.z(); });

  auto milling_path = MillingPath{milling_points, cutter_};
  GCodeSerializer::serializePath(milling_path,
                                 intersectionCurve.getName() + ".k08");
}

std::vector<DetailedPathGenerator::Coord>
DetailedPathGenerator::shortestPathAlongIntersectioCurve(
    Coord start, Coord end, const IntersectionTexture &intersectionTexture,
    Direction direction) const {
  std::vector<Coord> coords;

  /// assume vertical for now
  if (direction == Direction::Horizontal) {
    return coords;
  }

  auto current_coord = start;

  while (current_coord != end) {
    coords.push_back(current_coord);
    for (int i = 0; i < 2; ++i) {
      bool found = false;
      if (found) {
        break;
      }

      for (int j = 0; j < 2; ++j) {
        Coord new_coord{.x = current_coord.x + i, .y = current_coord.y + j};
        if (intersects(new_coord, intersectionTexture)) {
          current_coord = new_coord;
          found = true;
          break;
        }
      }
    }
  }

  return coords;
}

bool DetailedPathGenerator::intersects(
    Coord coord, const IntersectionTexture &intersectionTexture) const {
  return intersectionTexture.getCellType(coord.x, coord.y) ==
         IntersectionTexture::CellType::Intersection;
}

std::vector<algebra::Vec3f> DetailedPathGenerator::safeConnectionBridge(
    Coord start, Coord end, const BezierSurface &surface) const {

  const auto &intersection_texture = surface.getIntersectionTexture();
  const auto offset_surface = algebra::NormalOffsetSurface(
      &surface.getAlgebraSurfaceC0(), cutter_.radius());

  // Helper to get texture bounds
  const int width = intersection_texture.getSize().width;
  const int height = intersection_texture.getSize().height;

  // 1. Get Wrapping Configuration
  // Assuming 0 = U (X-axis) and 1 = V (Y-axis)
  const bool wrap_x = surface.wrapped(0);
  const bool wrap_y = surface.wrapped(1);

  auto safe_coord = [&](Coord coord) {
    return !intersection_texture.isTrimmed(coord.x, coord.y);
  };

  std::unordered_set<Coord> visited;
  std::unordered_map<Coord, Coord> prev;
  std::queue<Coord> queue;

  std::array<Coord, 8> d{Coord{.x = -1, .y = 0},  Coord{.x = 1, .y = 0},
                         Coord{.x = 0, .y = -1},  Coord{.x = 0, .y = 1},
                         Coord{.x = -1, .y = -1}, Coord{.x = -1, .y = 1},
                         Coord{.x = 1, .y = -1},  Coord{.x = 1, .y = 1}};

  // BFS INITIALIZATION
  queue.push(start);
  visited.insert(start);

  bool path_found = false;

  while (!queue.empty()) {
    auto current_coord = queue.front();
    queue.pop();

    if (current_coord == end) {
      path_found = true;
      break;
    }

    for (const auto &d_coord : d) {
      // Calculate theoretical next position
      int nx = current_coord.x + d_coord.x;
      int ny = current_coord.y + d_coord.y;

      // 2. Handle X Wrapping
      if (wrap_x) {
        // C++ % operator can return negative for negative numbers,
        // so we handle the wrap manually or use ((n % M) + M) % M
        if (nx < 0)
          nx = width - 1;
        else if (nx >= width)
          nx = 0;
      } else {
        // Standard Boundary Check
        if (nx < 0 || nx >= width)
          continue;
      }

      // 3. Handle Y Wrapping
      if (wrap_y) {
        if (ny < 0)
          ny = height - 1;
        else if (ny >= height)
          ny = 0;
      } else {
        // Standard Boundary Check
        if (ny < 0 || ny >= height)
          continue;
      }

      Coord next_coord = {.x = nx, .y = ny};

      // Standard BFS Logic
      if (visited.contains(next_coord)) {
        continue;
      }

      if (safe_coord(next_coord)) {
        visited.insert(next_coord);
        prev.insert({next_coord, current_coord});
        queue.push(next_coord);
      }
    }
  }

  if (!path_found) {
    return {};
  }

  /// Reconstruct path
  std::vector<Coord> path;
  auto current_coord = end;

  path.push_back(end);

  while (current_coord != start) {
    current_coord = prev.at(current_coord);
    path.push_back(current_coord);
  }
  std::ranges::reverse(path);

  // Convert to 3D Points
  std::vector<algebra::Vec3f> points;
  points.reserve(path.size());

  for (const auto &coord : path) {
    auto point =
        offset_surface.value(intersection_texture.uv(coord.x, coord.y));
    point.y() -= cutter_.radius();
    point.y() += 1.5f;
    points.push_back(point);
  }

  fixIntersectionLine(points, surface);

  return points;
}

void DetailedPathGenerator::fixIntersectionLine(
    std::vector<algebra::Vec3f> &points, const BezierSurface &surface) const {
  auto fix_point = [&](algebra::Vec3f &point) {
    auto index = heightMap_->posToIndex(point);
    auto normal = heightMap_->normalAtIndex(index);
    auto safe_height = heightMap_->findMinimumSafeHeightForCut(point, cutter_);
    const float kGougeTolerance = 0.05f;

    if (std::isnan(safe_height)) {
      return;
    }
    if (point.y() >= safe_height - 1.5f - kGougeTolerance) {
      return;
    }

    std::println("point.y() == {}, safe_height == {}", point.y(),
                 safe_height - 1.5f);

    auto dist = safe_height - point.y() - 1.5f;

    point.y() = safe_height - 1.5f; // point + normal * (dist);
    return;
  };

  for (auto &p : points) {
    fix_point(p);
  }
};

std::vector<algebra::Vec3f>
DetailedPathGenerator::createSafeBridge(Coord start, Coord end,
                                        const BezierSurface &surface) const {
  std::vector<algebra::Vec3f> bridge_points;
  const auto &tex = surface.getIntersectionTexture();
  // 1. Bresenham Algorithm Setup
  int x0 = start.x;
  int y0 = start.y;
  int x1 = end.x;
  int y1 = end.y;

  int dx = std::abs(x1 - x0);
  int dy = -std::abs(y1 - y0);
  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  while (true) {
    if (x0 >= 0 && y0 >= 0 && x0 < heightMap_->divisions_.x_ &&
        y0 < heightMap_->divisions_.z_) {

      // uint32_t index = heightMap_->globalIndex(x0, y0);
      // algebra::Vec3f world_pos = heightMap_->indexToPos(index);
      auto pos = surface.value(tex.uv(x0, y0));
      // float safe_y =
      //      heightMap_->findMinimumSafeHeightForCut(pos, cutter_) - 1.5;
      //  pos.y() = safe_y;

      bridge_points.push_back(pos);
    }

    if (x0 == x1 && y0 == y1) {
      break;
    }

    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }

  // 3. Optimization (Optional but Recommended)
  // Bresenham creates a point for every pixel. This is very dense.
  // reducing the points keeps the shape but removes redundant collinear
  // points. Uses Plane::XZ because the path is primarily horizontal, but
  // height varies.
  fixIntersectionLine(bridge_points, surface);
  return bridge_points;
  // return algebra::RDP::reducePoints(bridge_points, 0.005f,
  // algebra::Plane::XZ);
}