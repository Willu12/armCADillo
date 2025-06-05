#pragma once

#include "gregorySurface.hpp"
#include "borderGraph.hpp"
#include "pointEntity.hpp"
#include <cstdint>
#include <functional>
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
  auto intersectionPoints = borderGraph.findHoles();

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
