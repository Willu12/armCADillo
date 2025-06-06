#include "gregorySurface.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "borderGraph.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
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

  const auto borderGraph = createBorderGraph(surfaces);

  auto cornerEdges = borderGraph.findHoles();
  // function that will create gregorySurfaces based on intersection points;
}

BorderGraph GregorySurface::createBorderGraph(
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces)
    const {
  std::vector<Border> borders;
  borders.reserve(surfaces.size());

  for (const auto &surface : surfaces) {
    borders.push_back(getBorder(surface));
  }
  return BorderGraph(borders);
}

Border GregorySurface::getBorder(const BezierSurfaceC0 &surface) const {
  std::vector<std::reference_wrapper<const PointEntity>> borderPoints;
  const auto &points = surface.getPointsReferences();
  std::unordered_map<PointRefPair, Edge, RefPairHash, RefPairEqual>
      pointsEdgeMap;
  borderPoints.reserve((surface.getPatches().sCount + 1) *
                       (surface.getPatches().tCount + 1));
  for (uint32_t u = 0; u < surface.getColCount(); u += 3)
    for (uint32_t v = 0; v < surface.getRowCount(); v += 3) {
      borderPoints.push_back(points[u * surface.getColCount() + v]);
      if (u == surface.getColCount() - 1 || v == surface.getRowCount() - 1)
        continue;
      Edge rightEdge{points[u * surface.getColCount() + v],
                     points[u * surface.getColCount() + v + 1],
                     points[u * surface.getColCount() + v + 2],
                     points[u * surface.getColCount() + v + 3]};
      Edge downEdge{points[u * surface.getColCount() + v],
                    points[(u + 1) * surface.getColCount() + v],
                    points[(u + 2) * surface.getColCount() + v],
                    points[(u + 3) * surface.getColCount() + v]};

      pointsEdgeMap.insert(
          {std::tie(rightEdge._points[0], rightEdge._points[3]), rightEdge});
      pointsEdgeMap.insert(
          {std::tie(downEdge._points[0], downEdge._points[3]), downEdge});
    }

  return Border{
      .points_ = borderPoints,
      .pointsEdgeMap_ = pointsEdgeMap,
      .uLen = surface.getPatches().sCount,
      .vLen = surface.getPatches().tCount,
  };
}

BezierSurfaceC0 &GregorySurface::findSurfaceForEdge(
    const PointEntity &p1, const PointEntity &p2,
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces)
    const {
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

std::array<GregoryQuad, 3> GregorySurface::GetGregoryQuadsFromEdges(
    const std::array<std::reference_wrapper<const Edge>, 3> &edges) const {
  const auto &p0 = edges[0].get()._points[0].get().getPosition();
  const auto &p1 = edges[1].get()._points[0].get().getPosition();
  const auto &p2 = edges[2].get()._points[0].get().getPosition();

  const auto centerPos = (p0 + p1 + p2) / 3.f;

  // create first point
  const auto &p01 = (p0 + p1) / 2.f;
  const auto &p12 = (p1 + p2) / 2.f;
  const auto &p02 = (p2 + p0) / 2.f;

  std::array<algebra::Vec3f, 4> top = {p01, 2.f / 3.f * p01 + centerPos / 3.f,
                                       1.f / 3.f * p01 + centerPos * 2.f / 3.f,
                                       centerPos};

  std::array<algebra::Vec3f, 4> bottom = {
      p0, 2.f / 3.f * p0 + p02 / 3.f, 1.f / 3.f * p0 + p02 * 2.f / 3.f, p02};

  std::array<algebra::Vec3f, 2> topSides = {p01 * 2.f / 3.f + p0 / 3.f,
                                            centerPos * 2.f / 3.f + p02 / 3.f};

  std::array<algebra::Vec3f, 2> bottomSides = {
      p01 / 3.f + p0 * 2.f / 3.f, centerPos / 3.f + p02 * 2.f / 3.f};

  // how to calcluatge tangent points;
  std::array<algebra::Vec3f, 4> uInner = {
      (p0 + p01 + centerPos) / 3.f, // between p0-p01-center
      (p1 + p01 + centerPos) / 3.f, // between p1-p01-center
      (p2 + p12 + centerPos) / 3.f, // between p2-p12-center
      (p0 + p02 + centerPos) / 3.f  // between p0-p02-center
  };

  std::array<algebra::Vec3f, 4> vInner = {
      (p0 + centerPos + p01) / 3.f, (p1 + centerPos + p12) / 3.f,
      (p2 + centerPos + p02) / 3.f, (p0 + centerPos + p01) / 3.f};

  GregoryQuad patch1 = {
      .top = top,
      .bottom = bottom,
      .bottomSides = bottomSides,
      .uInner = uInner,
      .vInner = vInner,
  };
}