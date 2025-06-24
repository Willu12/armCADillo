#pragma once

#include "IGroupedEntity.hpp"
#include "IMeshable.hpp"
#include "ISubscriber.hpp"
#include "bezierSurfaceMesh.hpp"
#include "intersectable.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <memory>
#include <vector>

struct Patches {
  uint32_t colCount;
  uint32_t rowCount;
};

class BezierSurface : public IGroupedEntity,
                      public ISubscriber,
                      public Intersectable {
public:
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

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurface(*this);
  };

  void update() override {
    _polyMesh = createPolyMesh();
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

  static inline float bernstein(int i, int n, float t) {
    float coeff = 1.0f;
    for (int k = 0; k < i; ++k) {
      coeff *= static_cast<float>(n - k) / (k + 1);
    }

    return coeff * std::pow(t, i) * std::pow(1.0f - t, n - i);
  }

protected:
  std::vector<std::reference_wrapper<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  std::unique_ptr<Mesh> _polyMesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
  Patches _patches = Patches{.colCount = 1, .rowCount = 1};
  bool _wireframe = false;

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