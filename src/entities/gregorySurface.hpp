#pragma once

#include "IEntity.hpp"
#include "IMeshable.hpp"
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
  const IMeshable &getMesh() const override { return *_mesh[0]; };
  const std::array<std::unique_ptr<BezierSurfaceMesh>, 3> &getMeshes() const {
    return _mesh;
  }
  explicit GregorySurface(const std::array<GregoryQuad, 3> &gregoryPatches);

private:
  std::array<GregoryQuad, 3> _gregoryPatches;
  std::array<std::unique_ptr<BezierSurfaceMesh>, 3> _mesh;
  inline static int kClassId;
  std::vector<std::array<algebra::Vec3f, 16>> calculateGregoryPoints();
  void updateMesh() override { _mesh = generateMesh(); };
  std::array<std::unique_ptr<BezierSurfaceMesh>, 3> generateMesh();

  static BorderGraph createBorderGraph(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  static std::array<GregoryQuad, 3>
  calculateGregoryPatchesForHole(const std::array<BorderEdge, 3> &edges);
};