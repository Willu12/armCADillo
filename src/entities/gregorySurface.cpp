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

  // 1. Get corner points and adjacent surfaces/inner points
  std::array<algebra::Vec3f, 3> corners;
  std::array<const BezierSurfaceC0 *, 3> adjSurfaces;
  std::array<std::array<algebra::Vec3f, 4>, 3> innerPoints;

  for (int i = 0; i < 3; ++i) {
    const Edge &edge = edges[i];
    corners[i] = edge._points[0].get().getPosition();
    adjSurfaces[i] = &findSurfaceForEdge(edge, surfaces);

    auto inner = findInnerPointsForEdge(edge, *adjSurfaces[i]);
    if (!inner) {
      throw std::runtime_error(
          "Could not find inner control points for an edge.");
    }
    innerPoints[i] = *inner;
  }

  // 2. Define the center point of the hole
  const auto centerPos = (corners[0] + corners[1] + corners[2]) / 3.0f;

  // 3. Calculate internal "Gregory points" for C1 continuity across hole
  // boundaries
  std::array<algebra::Vec3f, 3> g_plus;
  std::array<algebra::Vec3f, 3> g_minus;

  for (int i = 0; i < 3; ++i) {
    const int next_i = (i + 1) % 3;
    const auto &p_i = corners[i];
    const auto &p_next = corners[next_i];

    // Sum of vectors from corners to center

    g_plus[i] = p_i + (p_i - centerPos) / 2.0f;
    g_minus[next_i] = p_next + (p_next - centerPos) / 2.0f;
  }

  // 4. Construct each of the 3 Gregory Quads
  for (int i = 0; i < 3; ++i) {
    int prev_i = (i + 2) % 3; // (i-1+3)%3
    const Edge &edge = edges[i];

    // Corner points for this quad: P_i, P_{i+1}, center, center
    const auto &p0 = corners[i];
    const auto &p1 = corners[(i + 1) % 3];

    // Boundary points along the original surface edge
    const auto &e0 = edge._points[0].get().getPosition(); // same as p0
    const auto &e1 = edge._points[1].get().getPosition();
    const auto &e2 = edge._points[2].get().getPosition();
    const auto &e3 = edge._points[3].get().getPosition(); // same as p1

    // Interior points from the adjacent surface
    const auto &i0 = innerPoints[i][0];
    const auto &i1 = innerPoints[i][1];
    const auto &i2 = innerPoints[i][2];
    const auto &i3 = innerPoints[i][3];

    // Calculate the "f" points (the interior Gregory points for C1 continuity)
    // These ensure the patch is smooth with the adjacent surface
    algebra::Vec3f f0 = (e0 + e1 + i0 + i1) / 4.0f;
    algebra::Vec3f f1 = (e2 + e3 + i2 + i3) / 4.0f;

    // Map to your GregoryQuad structure
    // This mapping depends on how you define your quad's orientation.
    // Let's assume 'bottom' is the edge adjacent to the original surface
    // and 'top' is the edge collapsing to the center point.
    quads[i].bottom = {p0, e1, e2, p1};

    // The top "edge" is just the center point
    quads[i].top = {centerPos, centerPos, centerPos, centerPos};

    // Sides connecting bottom to top
    quads[i].bottomSides = {p0, p1}; // These names are confusing, remapping
    quads[i].topSides = {centerPos, centerPos};

    // The crucial interior points
    // uInner and vInner represent the "split" points that make Gregory patches
    // work. They allow C1 continuity without requiring C2.
    quads[i].uInner[0] = f0;
    quads[i].uInner[1] = f1;
    quads[i].uInner[2] = g_plus[i];
    quads[i].uInner[3] = g_minus[(i + 1) % 3];

    // vInner is calculated to ensure tangent plane continuity
    quads[i].vInner[0] = f0;
    quads[i].vInner[1] = f1;
    quads[i].vInner[2] = g_plus[i];
    quads[i].vInner[3] = g_minus[(i + 1) % 3];
  }

  // Correction for the interior points to ensure smoothness across the new
  // patch boundaries
  for (int i = 0; i < 3; ++i) {
    int next_i = (i + 1) % 3;

    // The two inner points must be collinear with the corner
    auto &g_p_i = quads[i].uInner[2];    // Corresponds to g_plus[i]
    auto &g_m_next = quads[i].uInner[3]; // Corresponds to g_minus[next_i]

    auto &g_p_next = quads[next_i].uInner[2]; // Corresponds to g_plus[next_i]
    auto &g_m_i = quads[i].vInner[3];         // A point from the other side

    // This is a simplified blending scheme. More complex ones exist.
    // For now, we will just use the calculated g_plus and g_minus.
    // The previous calculation is a decent starting point.
  }

  return quads;
}

std::vector<std::array<algebra::Vec3f, 16>>
GregorySurface::calculateGregoryPoints() {
  std::vector<std::array<algebra::Vec3f, 16>> bezierPatches;
  bezierPatches.reserve(3);

  for (const auto &quad : _gregoryPatches) {
    std::array<algebra::Vec3f, 16> points;

    // The 12 boundary points are the same
    points[0] = quad.bottom[0];
    points[1] = quad.bottom[1];
    points[2] = quad.bottom[2];
    points[3] = quad.bottom[3];

    // Left and Right sides (mapping might need adjustment based on your
    // convention)
    points[4] = quad.bottomSides[0]; // This mapping seems wrong in your struct
    points[7] = quad.topSides[0];    // Let's assume a standard 4x4 grid.
    points[8] = quad.bottomSides[1];
    points[11] = quad.topSides[1];

    // A standard mapping for a 4x4 grid [row * 4 + col]
    // Row 0
    points[0] = quad.bottom[0];
    points[1] = quad.bottom[1];
    points[2] = quad.bottom[2];
    points[3] = quad.bottom[3];
    // Row 3
    points[12] = quad.top[0];
    points[13] = quad.top[1];
    points[14] = quad.top[2];
    points[15] = quad.top[3];
    // Column 0
    points[4] = (quad.vInner[0] + quad.uInner[0]) /
                2.0f; // Placeholder, need side points
    points[8] = (quad.vInner[3] + quad.uInner[3]) / 2.0f; // Placeholder
    // Column 3
    points[7] = (quad.vInner[1] + quad.uInner[1]) / 2.0f;  // Placeholder
    points[11] = (quad.vInner[2] + quad.uInner[2]) / 2.0f; // Placeholder

    // THE 4 INNER BEZIER POINTS are the average of the Gregory "split" points
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

      // ↓ Bottom edge
      if (match(u, v, 1, 0) || matchReversed(u, v, 1, 0)) {
        return std::array{points[idx(1, 1)].get().getPosition(),
                          points[idx(2, 1)].get().getPosition(),
                          points[idx(0, 1)].get().getPosition(),
                          points[idx(3, 1)].get().getPosition()};
      }

      // ↑ Top edge
      if (match(u, v + 3, 1, 0) || matchReversed(u, v + 3, 1, 0)) {
        return std::array{points[idx(1, 2)].get().getPosition(),
                          points[idx(2, 2)].get().getPosition(),
                          points[idx(0, 2)].get().getPosition(),
                          points[idx(3, 2)].get().getPosition()};
      }

      // ← Left edge
      if (match(u, v, 0, 1) || matchReversed(u, v, 0, 1)) {
        return std::array{points[idx(1, 1)].get().getPosition(),
                          points[idx(1, 2)].get().getPosition(),
                          points[idx(1, 0)].get().getPosition(),
                          points[idx(1, 3)].get().getPosition()};
      }

      // → Right edge
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

std::unique_ptr<BezierSurfaceMesh> GregorySurface::generateMesh() {
  auto _points = calculateGregoryPoints()[0];
  std::vector<float> controlPointsPositions(_points.size() * 3);

  for (const auto &[i, point] : _points | std::views::enumerate) {
    controlPointsPositions[3 * i] = point[0];
    controlPointsPositions[3 * i + 1] = point[1];
    controlPointsPositions[3 * i + 2] = point[2];
  }
  return BezierSurfaceMesh::create(controlPointsPositions, 1, 1);
}