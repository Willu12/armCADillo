#pragma once

#include "IEntity.hpp"
#include "ISubscriber.hpp"
#include "bezierSurfaceMesh.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"

struct Patches {
  uint32_t sCount;
  uint32_t tCount;
};

class BezierSurface : public IEntity, public ISubscriber {
public:
  const MeshDensity &getMeshDensity() const { return _meshDensity; }
  MeshDensity &getMeshDensity() { return _meshDensity; }

  const Patches &getPatches() const { return _patches; }
  const IMeshable &getMesh() const override { return *_mesh; }
  std::vector<std::shared_ptr<IEntity>> getPoints() {
    std::vector<std::shared_ptr<IEntity>> points;
    points.reserve(_points.size());
    for (const auto &point : _points) {
      points.emplace_back(point);
    }
    return points;
  }
  const bool &wireframe() const { return _wireframe; }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurface(*this);
  };

  void update() override { updateMesh(); }
  void onSubscribableDestroyed(ISubscribable &publisher) override {}

protected:
  std::vector<std::shared_ptr<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
  Patches _patches = Patches{.sCount = 1, .tCount = 1};
  bool _wireframe = true;
};