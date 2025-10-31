#include "gregorySurface.hpp"
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
#include <ranges>
#include <vector>

std::vector<std::unique_ptr<GregorySurface>>
GregorySurface::createGregorySurfaces(
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  std::vector<std::unique_ptr<GregorySurface>> gregory_surfaces;

  const auto border_graph = createBorderGraph(surfaces);
  auto holes = border_graph.findHoles();

  if (holes.empty()) {
    return gregory_surfaces;
  }

  for (auto &hole : holes) {
    gregory_surfaces.emplace_back(std::make_unique<GregorySurface>(hole));
  }

  return gregory_surfaces;
}

GregorySurface::GregorySurface(const std::array<BorderEdge, 3> &edges)
    : _edges(edges) {

  for (auto &edge : _edges) {
    for (auto &p : edge._edge._points) {
      subscribe(p.get());
    }
    for (auto &p : edge._innerEdge._points) {
      subscribe(p.get());
    }
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
    borders.emplace_back(surface);
  }
  return BorderGraph(borders);
}

void GregorySurface::createGregoryPatches() {
  std::array<std::array<algebra::Vec3f, 7>, 3> subdivided_edges;
  std::array<std::array<algebra::Vec3f, 7>, 3> subdivided_inner_edges;
  std::array<algebra::Vec3f, 3> p1;
  std::array<algebra::Vec3f, 3> p2;
  std::array<algebra::Vec3f, 3> p3;

  ccwOrderEdges(_edges);
  std::array<algebra::Vec3f, 3> q;
  for (const auto &[i, edge] : _edges | std::views::enumerate) {
    subdivided_edges[i] = _edges[i]._edge.subdivide();
    subdivided_inner_edges[i] = _edges[i]._innerEdge.subdivide();
    p3[i] = subdivided_edges[i][3];
    p2[i] = 2.f * subdivided_edges[i][3] - subdivided_inner_edges[i][3];
    q[i] = (3.f * p2[i] - p3[i]) / 2.f;
  }
  const auto p = (q[0] + q[1] + q[2]) / 3.f;
  for (const auto &[i, edge] : _edges | std::views::enumerate) {
    p1[i] = (2.f * q[i] + p) / 3.f;
  }

  auto &edge = subdivided_edges[0];
  auto &inner_edge = subdivided_inner_edges[0];

  auto &left_edge = subdivided_edges[2];
  auto &left_inner_edge = subdivided_inner_edges[2];

  auto &right_edge = subdivided_edges[1];
  auto &right_inner_edge = subdivided_inner_edges[1];

  auto v = p1[2] - p;
  auto w = edge[2] - edge[3];
  auto z = left_edge[4] - left_edge[3];
  auto y = p1[0] - p;
  auto x = right_edge[2] - right_edge[3];
  GregoryQuad quad1{.top{left_edge[3], p2[2], p1[2], p},
                    .bottom = {edge[0], edge[1], edge[2], edge[3]},
                    .topSides{left_edge[4], p1[0]},    // p1[0]},
                    .bottomSides{left_edge[5], p2[0]}, // P2[0]},
                    .uInner{
                        2.f * left_edge[4] - left_inner_edge[4],
                        2.f * left_edge[5] - left_inner_edge[5],
                        p2[0] + 1.f / 3.f * v + w * 2.f / 3.f,
                        p1[0] + 2.f / 3.f * v + w / 3.f,
                    },
                    .vInner{
                        p2[2] + 2.f / 3.f * z + y / 3.f,
                        2.f * edge[1] - inner_edge[1],
                        2.f * edge[2] - inner_edge[2],

                        p1[2] + 1.f / 3.f * z + y * 2.f / 3.f,
                    }};
  GregoryQuad quad2{
      .top{p, p1[1], p2[1], right_edge[3]},
      .bottom = {edge[3], edge[4], edge[5], edge[6]},
      .topSides{p1[0], right_edge[2]},
      .bottomSides{p2[0], right_edge[1]},
      .uInner{p1[0] + 1.f / 3.f * (edge[4] - edge[3]) + 2.f / 3.f * (p1[1] - p),
              p2[0] + 2.f / 3.f * (edge[4] - edge[3]) + 1.f / 3.f * (p1[1] - p),
              2.f * right_edge[1] - right_inner_edge[1],
              2.f * right_edge[2] - right_inner_edge[2]},
      .vInner{
          p1[1] + 2.f / 3.f * y + x * 1.f / 3.f,
          2.f * edge[4] - inner_edge[4],
          2.f * edge[5] - inner_edge[5],
          p2[1] + 1.f / 3.f * y + x * 2.f / 3.f,

      }};

  GregoryQuad quad3{
      .top{right_edge[6], right_edge[5], right_edge[4], right_edge[3]},
      .bottom{left_edge[3], p2[2], p1[2], p},
      .topSides{left_edge[1], p2[1]},
      .bottomSides{left_edge[2], p1[1]},
      .uInner{
          2.f * left_edge[1] - left_inner_edge[1],
          2.f * left_edge[2] - left_inner_edge[2],
          p1[1] + 2.f / 3.f * (p1[2] - p) +
              (right_edge[4] - right_edge[3]) / 3.f,
          p2[1] + 1.f / 3.f * (p1[2] - p) +
              (right_edge[4] - right_edge[3]) * 2.f / 3.f,
      },
      .vInner{
          2.f * right_edge[5] - right_inner_edge[5],
          p2[2] + 2.f / 3.f * (left_edge[2] - left_edge[3]) +
              (p1[1] - p) * 1.f / 3.f,
          p1[2] + 1.f / 3.f * (left_edge[2] - left_edge[3]) +
              (p1[1] - p) * 2.f / 3.f,
          2.f * right_edge[4] - right_inner_edge[4],
      }};

  _gregoryPatches = {quad1, quad2, quad3};
}

std::array<std::unique_ptr<GregoryMesh>, 3> GregorySurface::generateMesh() {
  std::array<std::unique_ptr<GregoryMesh>, 3> meshes;
  for (const auto &[i, quad] : _gregoryPatches | std::views::enumerate) {
    meshes[i] = GregoryMesh::create(quad);
  }
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