#include "detailedPathGenerator.hpp"
#include "bezierSurface.hpp"
#include "entitiesTypes.hpp"
#include "intersectionCurve.hpp"
#include "intersectionTexture.hpp"
#include "millingPath.hpp"
#include "normalOffsetSurface.hpp"
#include "plane.hpp"
#include "rdp.hpp"
#include "vec.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <sys/types.h>
#include <vector>

static constexpr float kFloorHeight = 0.f;
static constexpr float kFloorHeightPath = 1.5f;
static constexpr uint32_t kMaxIndex = std::numeric_limits<uint32_t>::max();

void DetailedPathGenerator::prepare() {

  /// TODO:
  /// 1. add proper interface for specyfing on which surfaces we want
  /// intersection.
  ///  2. add option to combine intersection textures.
  /// The result of this phase is that every surface of the model has proper
  /// intersection texture and can be processed

  /// for all pairs of surfaces
  for (int i = 0; i < model_->surfaces().size(); ++i) {
    for (int j = i + 1; j < model_->surfaces().size(); ++j) {

      auto *first_surface = model_->surfaces()[i];
      auto *second_surface = model_->surfaces()[j];

      auto first_algebra_surface = first_surface->getAlgebraSurfaceC0();
      auto second_algebra_surface = second_surface->getAlgebraSurfaceC0();

      offset_surfaces_[i] = std::make_unique<algebra::NormalOffsetSurface>(
          &first_algebra_surface, cutter_.diameter_ / 2.f);

      offset_surfaces_[j] = std::make_unique<algebra::NormalOffsetSurface>(
          &second_algebra_surface, cutter_.diameter_ / 2.f);

      intersectionFinder_->setSurfaces(offset_surfaces_[i].get(),
                                       offset_surfaces_[j].get());

      auto intersection = intersectionFinder_->find(false);
      if (!intersection) {
        continue;
      }

      std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>
          bounds(first_surface->bounds(), second_surface->bounds());

      auto intersection_curve = std::make_unique<IntersectionCurve>(
          *intersection, bounds, intersection->looped);

      intersection_curve->setFirstPoint(intersection->firstPoint);
      intersections_.push_back(intersection_curve.get());

      intersection_curve->getFirstTexture().setWrapping(
          first_surface->wrapped(0), first_surface->wrapped(1));
      intersection_curve->getSecondTexture().setWrapping(
          second_surface->wrapped(0), second_surface->wrapped(1));

      first_surface->combineIntersectionTexture(
          intersection_curve->getFirstTexturePtr());
      second_surface->combineIntersectionTexture(
          intersection_curve->getSecondTexturePtr());

      scene_->addEntity(EntityType::IntersectionCurve,
                        std::move(intersection_curve));
    }
  }
}
void DetailedPathGenerator::generate() {
  /// This section assumes that every model surface has proper intersection
  /// texture and all sections that should be trimmed are set.

  for (auto *surface : model_->surfaces()) {
    //// set floor
    setFloorAsTrimmed(*surface);
    auto segments = generateLineSegments(*surface, Direction::Vertical, 100);
    //  colorSegments(*surface, segments);
    auto surface_paths = generateSurfacePaths(*surface, segments);
    auto path = combineSurfacePaths(surface_paths);

    auto milling_path = MillingPath{path, cutter_};
    gCodeSerializer_->serializePath(milling_path, surface->getName() + ".k08");
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
DetailedPathGenerator::generateLineSegments(BezierSurface &surface,
                                            Direction direction,
                                            uint32_t lineCount) {
  std::vector<std::vector<Coord>> lines;
  lines.reserve(lineCount);

  const auto &tex = *surface.getIntersectionTexture();
  const auto size = tex.getSize();
  const uint32_t max_index = size.height;
  const uint32_t step = max_index / lineCount;

  for (uint32_t line = 0; line < max_index; line += step) {
    std::vector<Coord> segments;

    for (uint32_t i = 0; i < max_index; ++i) {
      uint32_t x = (direction == Direction::Horizontal) ? i : line;
      uint32_t y = (direction == Direction::Horizontal) ? line : i;

      bool curr_trim = tex.isTrimmed(x, y);

      if ((i == 0 || i == max_index - 1) && !curr_trim) {
        segments.emplace_back(x, y);
      }

      if (i < max_index - 1) {
        uint32_t x_next = (direction == Direction::Horizontal) ? i + 1 : line;
        uint32_t y_next = (direction == Direction::Horizontal) ? line : i + 1;

        bool next_trim = tex.isTrimmed(x_next, y_next);

        if (curr_trim != next_trim) {
          segments.emplace_back(x, y);
        }
      }
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

      /// TODO:
      /// Maybe go along the intersection curve till x/y are proper.

      if (reverse) {
        std::swap(start, end);
      }

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
    offset_point.y() += kFloorHeightPath;
    points.emplace_back(offset_point);

    current_coord.x += delta.x;
    current_coord.y += delta.y;
  }

  /// last_point
  auto uv = texture.uv(end.x, end.y);
  auto offset_point = offset_surface.value(uv);
  offset_point.y() += kFloorHeightPath;
  points.emplace_back(offset_point);

  if (reversed) {
    std::ranges::reverse(points);
  }

  algebra::Plane plane =
      start.x == end.x ? algebra::Plane::YZ : algebra::Plane::XZ;
  return algebra::RDP::reducePoints(points, 10e-5, plane);
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
