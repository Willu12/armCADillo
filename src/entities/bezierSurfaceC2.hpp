#pragma once
#include <array>
#include <ranges>
#include <sys/types.h>
#include <vector>

#include "bezierSurface.hpp"
#include "vec.hpp"
class BezierSurfaceC2 : public BezierSurface {
public:
  static std::shared_ptr<BezierSurfaceC2>
  createFlat(const algebra::Vec3f &position, uint32_t u_patches,
             uint32_t v_patches);

  static std::shared_ptr<BezierSurfaceC2>
  createCylinder(const algebra::Vec3f &position, float r, float h);

  void updateMesh() override {
    updateBezierSurface();
    _mesh = generateMesh();
  }
  void updateBezierSurface();
  static std::array<std::array<algebra::Vec3f, 4>, 4>
  processPatch(const std::array<std::array<algebra::Vec3f, 4>, 4> &patch);
  uint32_t getColCount() override { return 3 + _patches.tCount; }
  uint32_t getRowCount() override { return 3 + _patches.sCount; }

private:
  explicit BezierSurfaceC2(const std::vector<algebra::Vec3f> &positions);
  std::vector<algebra::Vec3f> _bezierControlPoints;
  inline static int _classId = 0;

  std::unique_ptr<BezierSurfaceMesh> generateMesh();
};