#pragma once

#include "IEntity.hpp"
#include "bezierSurfaceMesh.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"

class BezierSurface : public IEntity {
public:
protected:
  std::vector<std::unique_ptr<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
};