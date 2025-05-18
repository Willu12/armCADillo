#pragma once
#include "bezierSurface.hpp"
#include "bezierSurfaceMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <memory>
#include <ranges>
#include <vector>

class BezierSurfaceC0 : public BezierSurface {
public:
  static std::shared_ptr<BezierSurfaceC0>
  createFlat(const algebra::Vec3f &position, uint32_t u_patches,
             uint32_t v_patches);
  static std::shared_ptr<BezierSurfaceC0>
  createCylinder(const algebra::Vec3f &position, float r, float h);

  void updateMesh() override { _mesh = generateMesh(); }

private:
  explicit BezierSurfaceC0(const std::vector<algebra::Vec3f> &positions);
  inline static int _id = 0;
  std::unique_ptr<BezierSurfaceMesh> generateMesh();
};
