#pragma once

#include "IGroupedEntity.hpp"
#include "IMeshable.hpp"
#include "ISubscriber.hpp"
#include "bezierSurfaceMesh.hpp"
#include "intersectable.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

struct Patches {
  uint32_t colCount;
  uint32_t rowCount;
};

struct LocalBezierPatch {
  std::array<std::array<algebra::Vec3f, 4>, 4> patch;
  algebra::Vec2f localPos;
};

class BezierSurface : public IGroupedEntity,
                      public ISubscriber,
                      public Intersectable,
                      public algebra::IDifferentialParametricForm<2, 3> {
public:
  virtual ~BezierSurface() = default;
  const MeshDensity &getMeshDensity() const { return _meshDensity; }
  MeshDensity &getMeshDensity() { return _meshDensity; }

  const Patches &getPatches() const { return _patches; }
  const IMeshable &getMesh() const override { return *_mesh; }
  const IMeshable &getPolyMesh() const { return *_polyMesh; }
  std::vector<std::reference_wrapper<PointEntity>> getPoints() {
    return _points;
  }

  bool &wireframe() { return _wireframe; }
  const bool &wireframe() const { return _wireframe; }
  algebra::ConnectionType &getConnectionType() { return _connectionType; }
  const algebra::ConnectionType &getConnectionType() const {
    return _connectionType;
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurface(*this);
  };

  void markToUpdate() override { _dirty = true; }
  void update() override;
  void onSubscribableDestroyed(ISubscribable &publisher) override {}

  virtual uint32_t getColCount() const = 0;
  virtual uint32_t getRowCount() const = 0;
  std::vector<std::reference_wrapper<const PointEntity>>
  getPointsReferences() const override;

  std::vector<std::reference_wrapper<PointEntity>> &
  getPointsReferences() override {
    return _points;
  }

  bool wrapped(size_t dim) const override {
    return getAlgebraSurfaceC0().wrapped(dim);
  }
  std::array<algebra::Vec2f, 2> bounds() const override {
    return getAlgebraSurfaceC0().bounds();
  }
  algebra::Vec3f value(const algebra::Vec2f &pos) const override {
    return getAlgebraSurfaceC0().value(pos);
  }
  std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const override {
    return getAlgebraSurfaceC0().derivatives(pos);
  }

  algebra::Vec3f normal(const algebra::Vec2f &pos) const {
    auto derivates = derivatives(pos);
    return (derivates.first.cross(derivates.second)).normalize();
  }

  algebra::Matrix<float, 3, 2>
  jacobian(const algebra::Vec2f &pos) const override {
    return getAlgebraSurfaceC0().jacobian(pos);
  }
  algebra::BezierSurfaceC0 getBezierC0Patch() const;

protected:
  std::vector<std::reference_wrapper<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  std::unique_ptr<Mesh> _polyMesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
  Patches _patches = Patches{.colCount = 1, .rowCount = 1};
  bool _wireframe = false;
  algebra::ConnectionType _connectionType;
  std::unique_ptr<algebra::BezierSurfaceC0> _algebraSurfaceC0;

  virtual void updateAlgebraicSurfaceC0() = 0;
  const algebra::BezierSurfaceC0 &getAlgebraSurfaceC0() const {
    return *_algebraSurfaceC0;
  };

  std::unique_ptr<Mesh> createPolyMesh();
};