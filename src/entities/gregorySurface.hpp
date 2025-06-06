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
  explicit GregorySurface(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  const IMeshable &getMesh() const override { return *_mesh; };

private:
  std::array<GregoryQuad, 3> _gregoryPatches;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  std::vector<std::array<algebra::Vec3f, 16>> calculateGregoryPoints();
  void updateMesh() override { _mesh = generateMesh(); };

  Border getBorder(const BezierSurfaceC0 &surface) const;

  BorderGraph createBorderGraph(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces)
      const;
  BezierSurfaceC0 &
  findSurfaceForEdge(const PointEntity &p1, const PointEntity &p2,
                     const std::vector<std::reference_wrapper<BezierSurfaceC0>>
                         &surfaces) const;
  std::array<GregoryQuad, 3> calculateGregoryPatchesForHole(
      const std::array<std::reference_wrapper<const Edge>, 3> &edges,
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces)
      const;
  std::optional<std::array<algebra::Vec3f, 4>>
  findInnerPointsForEdge(const Edge &edge,
                         const BezierSurfaceC0 &surface) const;
  std::unique_ptr<BezierSurfaceMesh> generateMesh();
};