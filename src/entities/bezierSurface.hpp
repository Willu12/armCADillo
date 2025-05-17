#pragma once

#include "IEntity.hpp"
#include "bezierSurfaceMesh.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"

struct Patches {
  uint32_t sCount;
  uint32_t tCount;
};

class BezierSurface : public IEntity {
public:
  const MeshDensity &getMeshDensity() { return _meshDensity; }
  const Patches &getPatches() { return _patches; }
  const IMeshable &getMesh() const override { return *_mesh; }
  const std::vector<std::shared_ptr<IEntity>> getPoints() {
    std::vector<std::shared_ptr<IEntity>> points;
    points.reserve(_points.size());
    for (const auto &point : _points) {
      points.emplace_back(point);
    }
    return points;
  }
  const bool &wireframe() const { return _wireframe; }

protected:
  std::vector<std::shared_ptr<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
  Patches _patches = Patches{.sCount = 1, .tCount = 1};
  bool _wireframe = true;
};