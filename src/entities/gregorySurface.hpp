#pragma once

#include "IEntity.hpp"
#include "IMeshable.hpp"
#include "ISubscriber.hpp"
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

class GregorySurface : public IEntity, public ISubscriber {
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
  explicit GregorySurface(const std::array<BorderEdge, 3> &edges);
  std::array<MeshDensity, 3> &getMeshDensities() { return _meshDensities; }
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitGregorySurface(*this);
  };
  bool &showTangentVectors() { return _showTangentVectors; }
  void update() override {
    createGregoryPatches();
    updateMesh();
  }
  void onSubscribableDestroyed(ISubscribable &publisher) override {};

private:
  std::array<GregoryQuad, 3> _gregoryPatches;
  std::array<std::unique_ptr<GregoryMesh>, 3> _mesh;
  std::array<std::unique_ptr<Mesh>, 3> _tangetVectorsMeshes;
  std::array<MeshDensity, 3> _meshDensities;
  std::array<BorderEdge, 3> _edges;
  bool _showTangentVectors = false;
  inline static int kClassId;
  void updateMesh() override {
    _mesh = generateMesh();
    _tangetVectorsMeshes = generateTangentMesh();
  };
  std::array<std::unique_ptr<GregoryMesh>, 3> generateMesh();
  std::array<std::unique_ptr<Mesh>, 3> generateTangentMesh();

  static BorderGraph createBorderGraph(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);
  void createGregoryPatches();
  static void ccwOrderEdges(std::array<BorderEdge, 3> &edges);
};