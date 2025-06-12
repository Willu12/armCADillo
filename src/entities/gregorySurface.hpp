#pragma once

#include "IEntity.hpp"
#include "IMeshable.hpp"
#include "bezierSurfaceC0.hpp"
#include "bezierSurfaceMesh.hpp"
#include "borderGraph.hpp"
#include "graph.hpp"
#include "gregoryMesh.hpp"
#include "gregoryQuad.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <array>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class GregorySurface : public IEntity {
public:
  static std::vector<std::shared_ptr<GregorySurface>> createGregorySurfaces(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  const IMeshable &getMesh() const override;
  const std::array<std::unique_ptr<GregoryMesh>, 3> &getMeshes() const {
    return _mesh;
  }

  const std::array<std::unique_ptr<Mesh>, 3> &getTangentMeshes() const {
    return _tangetVectorsMeshes;
  }
  explicit GregorySurface(const std::array<GregoryQuad, 3> &gregoryPatches);
  std::array<MeshDensity, 3> &getMeshDensities() { return _meshDensities; }
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitGregorySurface(*this);
  };
  bool &showTangentVectors() { return _showTangentVectors; }

private:
  std::array<GregoryQuad, 3> _gregoryPatches;
  std::array<std::unique_ptr<GregoryMesh>, 3> _mesh;
  std::array<std::unique_ptr<Mesh>, 3> _tangetVectorsMeshes;
  std::array<MeshDensity, 3> _meshDensities;
  bool _showTangentVectors = false;
  inline static int kClassId;
  std::vector<std::array<algebra::Vec3f, 16>> calculateGregoryPoints();
  void updateMesh() override {
    _mesh = generateMesh();
    _tangetVectorsMeshes = generateTangentMesh();
  };
  std::array<std::unique_ptr<GregoryMesh>, 3> generateMesh();
  std::array<std::unique_ptr<Mesh>, 3> generateTangentMesh();

  static BorderGraph createBorderGraph(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  static std::array<GregoryQuad, 3>
  calculateGregoryPatchesForHole(std::array<BorderEdge, 3> &edges);
  static void ccwOrderEdges(std::array<BorderEdge, 3> &edges);
};