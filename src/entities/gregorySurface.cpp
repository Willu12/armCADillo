#pragma once

#include "gregorySurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "borderGraph.hpp"
#include "pointEntity.hpp"
#include <cstdint>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

std::pair<
    algebra::Graph,
    std::unordered_map<std::size_t, std::reference_wrapper<const PointEntity>>>
GregorySurface::createSurfacesGraph(
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces)
    const {
  // create Graph from points.
  std::vector<Border> borders;
  borders.reserve(surfaces.size());
  for (const auto &surface : surfaces)
    borders.push_back(
        Border{.points_ = std::as_const(surface.get()).getPointsReferences(),
               .uLen = surface.get().getColCount(),
               .vLen = surface.get().getRowCount()});
  BorderGraph borderGraph(borders);
  auto cornerPointsTriangles = borderGraph.findHoles();

  for (const auto &cornerPointsTriangle : cornerPointsTriangles) {
  }
  // function that will create gregorySurfaces based on intersection points;
}

std::vector<std::reference_wrapper<const PointEntity>>
GregorySurface::getBorder(const BezierSurfaceC0 &surface) const {
  std::vector<std::reference_wrapper<const PointEntity>> borderPoints;
  const auto &points = surface.getPointsReferences();
  borderPoints.reserve((surface.getPatches().sCount + 1) *
                       (surface.getPatches().tCount + 1));
  for (uint32_t u = 0; u < surface.getColCount(); u += 3)
    for (uint32_t v = 0; v < surface.getRowCount(); v += 3) {
      borderPoints.push_back(points[u * surface.getColCount() + v]);
    }
  return borderPoints;
}

BezierSurfaceC0 &GregorySurface::findSurfaceForEdge(
    const PointEntity &p1, const PointEntity &p2,
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  auto has_both_points = [&](BezierSurfaceC0 &surface) {
    auto points =
        surface.getPointsReferences() |
        std::ranges::views::transform(
            [](auto &ref) -> const PointEntity * { return &ref.get(); });
    return std::ranges::find(points, &p1) != points.end() &&
           std::ranges::find(points, &p2) != points.end();
  };

  auto it = std::ranges::find_if(
      surfaces, [&](auto &s) { return has_both_points(s.get()); });

  if (it != surfaces.end()) {
    return it->get();
  }
  throw std::runtime_error("No surface found that contains both given points.");
}
