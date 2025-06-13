#include "gregorySurface.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "borderGraph.hpp"
#include "gregoryMesh.hpp"
#include "gregoryQuad.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <numeric>
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

  for (auto &hole : holes) {
    gregorySurfaces.emplace_back(std::make_shared<GregorySurface>(hole));
  }
  return gregorySurfaces;
}

GregorySurface::GregorySurface(const std::array<BorderEdge, 3> &edges)
    : _edges(edges) {

  for (auto &edge : _edges) {
    for (auto &p : edge._edge._points)
      subscribe(p.get());
    for (auto &p : edge._innerEdge._points)
      subscribe(p.get());
  }
  createGregoryPatches();
  updateMesh();
  _id = kClassId++;
  _name = "GregorySurface" + std::to_string(_id);
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

void GregorySurface::createGregoryPatches() {
  std::array<std::array<algebra::Vec3f, 7>, 3> subdividedEdges;
  std::array<std::array<algebra::Vec3f, 7>, 3> subdividedInnerEdges;
  std::array<algebra::Vec3f, 3> P2;
  std::array<algebra::Vec3f, 3> P3;
  std::array<algebra::Vec3f, 3> P1;

  ccwOrderEdges(_edges);
  std::array<algebra::Vec3f, 3> Q;
  for (const auto &[i, edge] : _edges | std::views::enumerate) {
    subdividedEdges[i] = _edges[i]._edge.subdivide();
    subdividedInnerEdges[i] = _edges[i]._innerEdge.subdivide();
    P3[i] = subdividedEdges[i][3];
    P2[i] = 2.f * subdividedEdges[i][3] - subdividedInnerEdges[i][3];
    Q[i] = (3.f * P2[i] - P3[i]) / 2.f;
  }
  const auto P = (Q[0] + Q[1] + Q[2]) / 3.f;
  for (const auto &[i, edge] : _edges | std::views::enumerate)
    P1[i] = (2.f * Q[i] + P) / 3.f;

  auto &edge = subdividedEdges[0];
  auto &innerEdge = subdividedInnerEdges[0];

  auto &leftEdge = subdividedEdges[2];
  auto &leftInnerEdge = subdividedInnerEdges[2];

  auto &rightEdge = subdividedEdges[1];
  auto &rightInnerEdge = subdividedInnerEdges[1];

  auto v = P1[2] - P;
  auto w = edge[2] - edge[3];
  auto z = leftEdge[4] - leftEdge[3];
  auto y = P1[0] - P;
  auto x = rightEdge[2] - rightEdge[3];
  GregoryQuad quad1{.top{leftEdge[3], P2[2], P1[2], P},
                    .bottom = {edge[0], edge[1], edge[2], edge[3]},
                    .topSides{leftEdge[4], P1[0]},    // P1[0]},
                    .bottomSides{leftEdge[5], P2[0]}, // P2[0]},
                    .uInner{
                        2.f * leftEdge[4] - leftInnerEdge[4],
                        2.f * leftEdge[5] - leftInnerEdge[5],
                        P2[0] + 1.f / 3.f * v + w * 2.f / 3.f,
                        P1[0] + 2.f / 3.f * v + w / 3.f,
                    },
                    .vInner{
                        P2[2] + 2.f / 3.f * z + y / 3.f,
                        2.f * edge[1] - innerEdge[1],
                        2.f * edge[2] - innerEdge[2],

                        P1[2] + 1.f / 3.f * z + y * 2.f / 3.f,
                    }};
  GregoryQuad quad2{
      .top{P, P1[1], P2[1], rightEdge[3]},
      .bottom = {edge[3], edge[4], edge[5], edge[6]},
      .topSides{P1[0], rightEdge[2]},
      .bottomSides{P2[0], rightEdge[1]},
      .uInner{P1[0] + 1.f / 3.f * (edge[4] - edge[3]) + 2.f / 3.f * (P1[1] - P),
              P2[0] + 2.f / 3.f * (edge[4] - edge[3]) + 1.f / 3.f * (P1[1] - P),
              2.f * rightEdge[1] - rightInnerEdge[1],
              2.f * rightEdge[2] - rightInnerEdge[2]},
      .vInner{
          P1[1] + 2.f / 3.f * y + x * 1.f / 3.f,
          2.f * edge[4] - innerEdge[4],
          2.f * edge[5] - innerEdge[5],
          P2[1] + 1.f / 3.f * y + x * 2.f / 3.f,

      }};

  GregoryQuad quad3{
      .top{rightEdge[6], rightEdge[5], rightEdge[4], rightEdge[3]},
      .bottom{leftEdge[3], P2[2], P1[2], P},
      .topSides{leftEdge[1], P2[1]},
      .bottomSides{leftEdge[2], P1[1]},
      .uInner{
          2.f * leftEdge[1] - leftInnerEdge[1],
          2.f * leftEdge[2] - leftInnerEdge[2],
          P1[1] - 2.f / 3.f * y + (rightEdge[4] - rightEdge[3]) / 3.f,
          P2[1] - 1.f / 3.f * y + (rightEdge[4] - rightEdge[3]) * 2.f / 3.f,
      },
      .vInner{
          2.f * rightEdge[5] - rightInnerEdge[5],
          P2[2] + 2.f / 3.f * (leftEdge[2] - leftEdge[3]) - y * 1.f / 3.f,
          P1[2] + 1.f / 3.f * (leftEdge[2] - leftEdge[3]) - y * 2.f / 3.f,

          2.f * rightEdge[4] - rightInnerEdge[4],
      }};

  _gregoryPatches = {quad1, quad2, quad3};
}

std::array<std::unique_ptr<GregoryMesh>, 3> GregorySurface::generateMesh() {
  std::array<std::unique_ptr<GregoryMesh>, 3> meshes;
  for (const auto &[i, quad] : _gregoryPatches | std::views::enumerate)
    meshes[i] = GregoryMesh::create(quad);
  return meshes;
}

const IMeshable &GregorySurface::getMesh() const { return *_mesh[0]; };

void GregorySurface::ccwOrderEdges(std::array<BorderEdge, 3> &edges) {
  auto end0 = edges[0]._edge._points[3];

  auto e1 = edges[1];
  auto e2 = edges[2];
  if (edges[1]._edge._points[0].get().getId() == end0.get().getId()) {
    e1 = edges[1];
  } else if (edges[1]._edge._points[3].get().getId() == end0.get().getId()) {
    e1 = edges[1];
    std::ranges::reverse(e1._edge._points);
    std::ranges::reverse(e1._innerEdge._points);
  } else if (edges[2]._edge._points[0].get().getId() == end0.get().getId()) {
    e1 = edges[2];
    e2 = edges[1];
  } else if (edges[2]._edge._points[3].get().getId() == end0.get().getId()) {
    e1 = edges[2];
    e2 = edges[1];
    std::ranges::reverse(e1._edge._points);
    std::ranges::reverse(e1._innerEdge._points);
  }

  if (e2._edge._points[0].get().getId() != e1._edge._points[3].get().getId()) {
    std::ranges::reverse(e2._edge._points);
    std::ranges::reverse(e2._innerEdge._points);
  }

  edges[1] = e1;
  edges[2] = e2;
}

std::array<std::unique_ptr<Mesh>, 3> GregorySurface::generateTangentMesh() {
  std::array<std::unique_ptr<Mesh>, 3> meshes;
  for (const auto &[i, quad] : _gregoryPatches | std::views::enumerate) {

    std::vector<float> vertices = GregoryMesh::createMeshData(quad);
    std::vector<uint32_t> indices(16);

    indices[0] = 4;
    indices[1] = 12;

    indices[2] = 6;
    indices[3] = 13;

    indices[4] = 7;
    indices[5] = 14;

    indices[6] = 5;
    indices[7] = 15;

    indices[8] = 1;
    indices[9] = 16;

    indices[10] = 2;
    indices[11] = 19;

    indices[12] = 9;
    indices[13] = 17;

    indices[14] = 10;
    indices[15] = 18;

    meshes[i] = Mesh::create(vertices, indices);
  }
  return meshes;
}