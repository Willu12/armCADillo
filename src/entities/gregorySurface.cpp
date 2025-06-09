#include "gregorySurface.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "borderGraph.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <ranges>
#include <unordered_set>
#include <utility>
#include <vector>

std::vector<std::shared_ptr<GregorySurface>>
GregorySurface::createGregorySurfaces(
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  std::vector<std::shared_ptr<GregorySurface>> gregorySurfaces;
  const auto borderGraph = createBorderGraph(surfaces);
  auto holes = borderGraph.findHoles();
  if (holes.empty())
    return gregorySurfaces;

  for (const auto &hole : holes) {
    gregorySurfaces.emplace_back(std::make_shared<GregorySurface>(
        calculateGregoryPatchesForHole(hole, surfaces)));
  }
  return gregorySurfaces;
}

GregorySurface::GregorySurface(
    const std::array<GregoryQuad, 3> &gregoryPatches) {
  _gregoryPatches = gregoryPatches;
  updateMesh();
  _id = kClassId++;
  _name = &"GregorySurface"[_id];
}

BorderGraph GregorySurface::createBorderGraph(
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  std::vector<Border> borders;
  borders.reserve(surfaces.size());

  for (const auto &surface : surfaces) {
    borders.push_back(getBorder(surface));
  }
  return BorderGraph(borders);
}
Border GregorySurface::getBorder(const BezierSurfaceC0 &surface) {
  std::vector<std::reference_wrapper<const PointEntity>> borderPoints;
  const auto &points = surface.getPointsReferences();
  std::unordered_map<PointRefPair, Edge, RefPairHash, RefPairEqual>
      pointsEdgeMap;

  const uint32_t cols = surface.getColCount();
  const uint32_t rows = surface.getRowCount();

  // Top border (v = 0)
  for (uint32_t u = 0; u <= cols - 4; u += 3) {
    uint32_t base = u;
    borderPoints.push_back(points[base]);
    pointsEdgeMap.insert({{points[base], points[base + 3]},
                          Edge{points[base], points[base + 1], points[base + 2],
                               points[base + 3]}});
  }

  // Right border (u = cols - 1)
  for (uint32_t v = 0; v <= rows - 4; v += 3) {
    uint32_t base = (v * cols) + (cols - 1);
    borderPoints.push_back(points[base]);
    pointsEdgeMap.insert(
        {{points[base], points[base + 3 * cols]},
         Edge{points[base], points[base + cols], points[base + 2 * cols],
              points[base + 3 * cols]}});
  }

  // Bottom border (v = rows - 1)
  for (int32_t u = cols - 1; u >= 3; u -= 3) {
    uint32_t base = (rows - 1) * cols + u;
    borderPoints.push_back(points[base]);
    pointsEdgeMap.insert({{points[base], points[base - 3]},
                          Edge{points[base], points[base - 1], points[base - 2],
                               points[base - 3]}});
  }

  // Left border (u = 0)
  for (int32_t v = rows - 1; v >= 3; v -= 3) {
    uint32_t base = v * cols;
    borderPoints.push_back(points[base]);
    pointsEdgeMap.insert(
        {{points[base], points[base - 3 * cols]},
         Edge{points[base], points[base - cols], points[base - 2 * cols],
              points[base - 3 * cols]}});
  }

  return Border{
      .points_ = borderPoints,
      .pointsEdgeMap_ = pointsEdgeMap,
      .uLen = surface.getPatches().sCount,
      .vLen = surface.getPatches().tCount,
  };
}

BezierSurfaceC0 &GregorySurface::findSurfaceForEdge(
    const Edge &edge,
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  auto has_both_points = [&](BezierSurfaceC0 &surface) {
    const PointEntity &p1 = edge._points[0];
    const PointEntity &p2 = edge._points[3];

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

std::array<GregoryQuad, 3> GregorySurface::calculateGregoryPatchesForHole(
    const std::array<Edge, 3> &edges,
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  std::array<GregoryQuad, 3> quads;

  std::array<algebra::Vec3f, 3> corners;
  std::array<const BezierSurfaceC0 *, 3> adjSurfaces;
  std::array<std::array<algebra::Vec3f, 4>, 3> innerPoints;

  for (int i = 0; i < 3; ++i) {
    const Edge &edge = edges[i];
    corners[i] = edge._points[0].get().getPosition();
    adjSurfaces[i] = &findSurfaceForEdge(edge, surfaces);

    auto inner = findInnerPointsForEdge(edge, *adjSurfaces[i]);
    if (!inner)
      throw std::runtime_error(
          "Could not find inner control points for an edge.");

    innerPoints[i] = *inner;
  }

  std::array<algebra::Vec3f, 3> QiList;
  std::array<algebra::Vec3f, 3> P3List;
  std::array<algebra::Vec3f, 3> P2List;

  // const auto centerPos = (corners[0] + corners[1] + corners[2]) / 3.0f;
  for (int i = 0; i < 3; ++i) {
    const Edge &edge = edges[i];

    const auto &p0 = corners[i];
    const auto &p1 = corners[(i + 1) % 3];

    const auto &P3i = (p0 + p1) / 2.f; // midpoint of boundary edge

    const auto &B2 = innerPoints[i][2];
    const auto &B3 = innerPoints[i][3];

    const auto P2i = P3i - (B3 - B2) / 3.f;
    const auto Qi = 1.5f * P3i - 0.5f * P2i; // extrapolated helper point

    // Store Q for averaging later
    QiList[i] = Qi;
    P3List[i] = P3i;
    P2List[i] = P2i;
  }
  const auto P = (QiList[0] + QiList[1] + QiList[2]) / 3.f;

  for (int i = 0; i < 3; ++i) {
    const auto &p0 = corners[i];
    const auto &p1 = corners[(i + 1) % 3];

    const auto &P3i = P3List[i];
    const auto &P2i = P2List[i];
    const auto P1i = (2.f * P + P2i) / 3.f;

    // Fill the Gregory quad — Bezier patch layout
    quads[i].bottom = {p0, (2.f * p0 + p1) / 3.f, (p0 + 2.f * p1) / 3.f,
                       p1};            // cubic edge
    quads[i].top = {P3i, P2i, P1i, P}; // into the center, inner Bézier curve

    // Optional: for better control over surface, define sides:
    quads[i].bottomSides = {(2.f * p0 + P3i) / 3.f, (2.f * p1 + P3i) / 3.f};
    quads[i].topSides = {(P3i + 2.f * P) / 3.f, (P3i + 2.f * P) / 3.f};
  }

  // here calcuate inner points

  return quads;
}

std::vector<std::array<algebra::Vec3f, 16>>
GregorySurface::calculateGregoryPoints() {
  std::vector<std::array<algebra::Vec3f, 16>> bezierPatches;
  bezierPatches.reserve(3);

  for (const auto &quad : _gregoryPatches) {
    std::array<algebra::Vec3f, 16> points;

    points[0] = quad.bottom[0];
    points[1] = quad.bottomSides[0];
    points[2] = quad.topSides[0];
    points[3] = quad.top[0];

    points[4] = quad.bottom[1];
    points[5] = quad.uInner[0];
    points[6] = quad.uInner[1];
    points[7] = quad.top[1];

    points[8] = quad.bottom[2];
    points[9] = quad.vInner[0];
    points[10] = quad.vInner[1];
    points[11] = quad.top[2];

    points[12] = quad.bottom[3];
    points[13] = quad.bottomSides[1];
    points[14] = quad.topSides[1];
    points[15] = quad.top[3];

    points[5] = (quad.uInner[0] + quad.vInner[0]) / 2.0f;
    points[6] = (quad.uInner[1] + quad.vInner[1]) / 2.0f;
    points[9] = (quad.uInner[3] + quad.vInner[3]) / 2.0f;
    points[10] = (quad.uInner[2] + quad.vInner[2]) / 2.0f;

    bezierPatches.push_back(points);
  }
  return bezierPatches;
}

std::optional<std::array<algebra::Vec3f, 4>>
GregorySurface::findInnerPointsForEdge(const Edge &edge,
                                       const BezierSurfaceC0 &surface) {
  const auto &points = surface.getPointsReferences();
  const int rows = surface.getRowCount();
  const int cols = surface.getColCount();

  const auto &e = edge._points;

  auto match = [&](int u0, int v0, int du, int dv) -> bool {
    for (int i = 0; i < 4; ++i) {
      int u = u0 + du * i;
      int v = v0 + dv * i;
      if (e[i].get().getId() != points[u * cols + v].get().getId())
        return false;
    }
    return true;
  };

  auto matchReversed = [&](int u0, int v0, int du, int dv) -> bool {
    for (int i = 0; i < 4; ++i) {
      int u = u0 + du * (3 - i);
      int v = v0 + dv * (3 - i);
      if (e[i].get().getId() != points[u * cols + v].get().getId())
        return false;
    }
    return true;
  };

  for (int u = 0; u <= rows - 4; u += 3) {
    for (int v = 0; v <= cols - 4; v += 3) {
      auto idx = [&](int du, int dv) { return (u + du) * cols + (v + dv); };

      // Bottom edge
      if (match(u, v, 1, 0) || matchReversed(u, v, 1, 0)) {
        return std::array{points[idx(1, 1)].get().getPosition(),
                          points[idx(2, 1)].get().getPosition(),
                          points[idx(0, 1)].get().getPosition(),
                          points[idx(3, 1)].get().getPosition()};
      }

      // Top edge
      if (match(u, v + 3, 1, 0) || matchReversed(u, v + 3, 1, 0)) {
        return std::array{points[idx(1, 2)].get().getPosition(),
                          points[idx(2, 2)].get().getPosition(),
                          points[idx(0, 2)].get().getPosition(),
                          points[idx(3, 2)].get().getPosition()};
      }

      // Left edge
      if (match(u, v, 0, 1) || matchReversed(u, v, 0, 1)) {
        return std::array{points[idx(1, 1)].get().getPosition(),
                          points[idx(1, 2)].get().getPosition(),
                          points[idx(1, 0)].get().getPosition(),
                          points[idx(1, 3)].get().getPosition()};
      }

      // Right edge
      if (match(u + 3, v, 0, 1) || matchReversed(u + 3, v, 0, 1)) {
        return std::array{points[idx(2, 1)].get().getPosition(),
                          points[idx(2, 2)].get().getPosition(),
                          points[idx(2, 0)].get().getPosition(),
                          points[idx(2, 3)].get().getPosition()};
      }
    }
  }

  return std::nullopt;
}

std::array<std::unique_ptr<BezierSurfaceMesh>, 3>
GregorySurface::generateMesh() {
  std::array<std::unique_ptr<BezierSurfaceMesh>, 3> meshes;
  for (int i = 0; i < 3; ++i) {
    auto _points = calculateGregoryPoints()[i];
    std::vector<float> controlPointsPositions(_points.size() * 3);

    for (const auto &[i, point] : _points | std::views::enumerate) {
      controlPointsPositions[3 * i] = point[0];
      controlPointsPositions[3 * i + 1] = point[1];
      controlPointsPositions[3 * i + 2] = point[2];
    }
    meshes[i] = BezierSurfaceMesh::create(controlPointsPositions, 1, 1);
  }
  return meshes;
}