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
    gregorySurfaces.emplace_back(
        std::make_shared<GregorySurface>(calculateGregoryPatchesForHole(hole)));
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
    borders.push_back(Border(surface));
  }
  return BorderGraph(borders);
}

std::array<GregoryQuad, 3> GregorySurface::calculateGregoryPatchesForHole(
    const std::array<BorderEdge, 3> &edges) {
  std::array<std::array<algebra::Vec3f, 7>, 3> subdividedEdges;
  std::array<std::array<algebra::Vec3f, 7>, 3> subdividedInnerEdges;
  std::array<algebra::Vec3f, 3> P2;
  std::array<algebra::Vec3f, 3> P3;
  std::array<algebra::Vec3f, 3> P1;

  std::array<algebra::Vec3f, 3> Q;
  for (const auto &[i, edge] : edges | std::views::enumerate) {
    subdividedEdges[i] = edges[i]._edge.subdivide();
    subdividedInnerEdges[i] = edges[i]._innerEdge.subdivide();
    P3[i] = subdividedEdges[i][3];
    P2[i] = 2.f * subdividedEdges[i][3] - subdividedInnerEdges[i][3];
    Q[i] = (3.f * P2[i] - P3[i]) / 2.f;
  }
  const auto P = (Q[0] + Q[1] + Q[2]) / 3.f;
  for (const auto &[i, edge] : edges | std::views::enumerate)
    P1[i] = (2.f * Q[i] + P) / 3.f;

  auto &edge = subdividedEdges[0];
  auto &innerEdge = subdividedInnerEdges[0];

  auto &leftEdge = subdividedEdges[2];
  auto &leftInnerEdge = subdividedInnerEdges[2];

  auto &rightEdge = subdividedEdges[1];
  auto &rightInnerEdge = subdividedInnerEdges[1];

  auto v = P - P1[2];
  auto w = edge[2] - edge[3];
  auto z = leftEdge[2] - leftEdge[3];
  auto y = P1[0] - P;
  auto x = rightEdge[2] - rightEdge[3];
  GregoryQuad quad1{
      .top{leftEdge[3], P2[2], P1[2], P},
      .bottom = {edge[0], edge[1], edge[2], edge[3]},
      .topSides{leftEdge[2], P1[0]},
      .bottomSides{leftEdge[1], P2[0]},
      .uInner{2.f * leftEdge[1] - leftInnerEdge[1], 2.f / 3.f * z + y / 3.f,
              2.f / 3.f * v + w / 3.f, 2.f * edge[2] - innerEdge[2]},
      .vInner{2.f * edge[1] - innerEdge[1],
              2.f * leftEdge[2] - leftInnerEdge[2],
              1.f / 3.f * z + y * 2.f / 3.f, 1.f / 3.f * v + w * 2.f / 3.f}};

  GregoryQuad quad2{.top{P, P1[1], P2[1], rightEdge[3]},
                    .bottom = {edge[3], edge[4], edge[5], edge[6]},
                    .topSides{P1[0], rightEdge[2]},
                    .bottomSides{P2[0], rightEdge[1]},
                    .uInner{-1.f * quad1.vInner[3], 2.f / 3.f * y + x / 3.f,
                            2.f * rightEdge[2] - rightInnerEdge[2],
                            2.f * edge[5] - innerEdge[5]},
                    .vInner{
                        2.f * edge[4] - innerEdge[4],
                        -1.f * quad1.uInner[2],
                        1.f / 3.f * y + x * 2.f / 3.f,
                        2.f * rightEdge[1] - rightInnerEdge[1],
                    }};

  GregoryQuad quad3{
      .top{rightEdge[3], P2[1], P1[1], P},
      .bottom = {leftEdge[6], leftEdge[5], leftEdge[4], leftEdge[3]},
      .topSides{rightEdge[4], P1[2]},
      .bottomSides{rightEdge[5], P2[2]},
      .uInner{2.f * rightEdge[5] - rightInnerEdge[5], -1.f * quad2.vInner[2],
              -1.f * quad1.vInner[2], 2.f * leftEdge[4] - leftInnerEdge[4]},
      .vInner{2.f * leftEdge[5] - leftInnerEdge[5],
              2.f * rightEdge[4] - rightInnerEdge[4], -1.f * quad2.uInner[1],
              -1.f * quad1.uInner[2]}};

  return {quad1, quad2, quad3};
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
    points[7] = quad.top[1];

    points[8] = quad.bottom[2];
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