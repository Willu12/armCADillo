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
  bool &isCyllinder() { return _cyllinder; }
  const bool &isCyllinder() const { return _cyllinder; }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurface(*this);
  };

  void update() override {
    _polyMesh = createPolyMesh();
    updateAlgebraicSurfaceC0();
    updateMesh();
  }
  void
  onSubscribableDestroyed(ISubscribable &publisher) override { /* update();*/ }
  virtual uint32_t getColCount() const = 0;
  virtual uint32_t getRowCount() const = 0;
  std::vector<std::reference_wrapper<const PointEntity>>
  getPointsReferences() const override {
    std::vector<std::reference_wrapper<const PointEntity>> pointsReferences;
    pointsReferences.reserve(_points.size());
    for (const auto &point : _points) {
      pointsReferences.emplace_back(point);
    }
    return pointsReferences;
  }
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
  bool _cyllinder = false;
  std::unique_ptr<algebra::BezierSurfaceC0> _algebraSurfaceC0;

  virtual void updateAlgebraicSurfaceC0() = 0;
  const algebra::BezierSurfaceC0 &getAlgebraSurfaceC0() const {
    return *_algebraSurfaceC0;
  };

  std::unique_ptr<Mesh> createPolyMesh() {
    std::vector<float> vertices(3 * _points.size());

    for (int i = 0; i < _points.size(); ++i) {
      vertices[3 * i] = _points[i].get().getPosition()[0];
      vertices[3 * i + 1] = _points[i].get().getPosition()[1];
      vertices[3 * i + 2] = _points[i].get().getPosition()[2];
    }

    auto rowCount = getRowCount();
    auto colCount = getColCount();

    std::vector<unsigned int> indices;

    for (int row = 0; row < rowCount; ++row) {
      for (int col = 0; col < colCount - 1; ++col) {
        int a = row * colCount + col;
        int b = a + 1;
        indices.push_back(a);
        indices.push_back(b);
      }
    }

    for (int col = 0; col < colCount; ++col) {
      for (int row = 0; row < rowCount - 1; ++row) {
        int a = row * colCount + col;
        int b = a + colCount;
        indices.push_back(a);
        indices.push_back(b);
      }
    }
    return Mesh::create(vertices, indices);
  }
};