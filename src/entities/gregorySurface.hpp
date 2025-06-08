#pragma once

#include "IEntity.hpp"
#include "bezierSurfaceC0.hpp"
#include "bezierSurfaceMesh.hpp"
#include "borderGraph.hpp"
#include "graph.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <array>
#include <functional>
#include <unordered_map>
#include <vector>

struct GregoryQuad {
  std::array<algebra::Vec3f, 4> top;
  std::array<algebra::Vec3f, 4> bottom;
  std::array<algebra::Vec3f, 2> topSides;
  std::array<algebra::Vec3f, 2> bottomSides;
  std::array<algebra::Vec3f, 4> uInner;
  std::array<algebra::Vec3f, 4> vInner;
};
class GregorySurface : public IEntity {
public:
  static std::vector<std::shared_ptr<GregorySurface>> createGregorySurfaces(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  const IMeshable &getMesh() const override { return *_mesh; };
  explicit GregorySurface(const std::array<GregoryQuad, 3> &gregoryPatches);

private:
  std::array<GregoryQuad, 3> _gregoryPatches;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  std::vector<std::array<algebra::Vec3f, 16>> calculateGregoryPoints();
  void updateMesh() override { _mesh = generateMesh(); };
  std::unique_ptr<BezierSurfaceMesh> generateMesh();

  static Border getBorder(const BezierSurfaceC0 &surface);

  static BorderGraph createBorderGraph(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  static BezierSurfaceC0 &findSurfaceForEdge(
      const Edge &edge,
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  static std::array<GregoryQuad, 3> calculateGregoryPatchesForHole(
      const std::array<Edge, 3> &edges,
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  static std::optional<std::array<algebra::Vec3f, 4>>
  findInnerPointsForEdge(const Edge &edge, const BezierSurfaceC0 &surface);
};