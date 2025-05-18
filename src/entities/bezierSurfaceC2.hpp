#pragma once
#include <ranges>
#include <vector>

#include "bezierSurface.hpp"
#include "vec.hpp"
class BezierSurfaceC2 : public BezierSurface {
public:
  static std::shared_ptr<BezierSurfaceC2>
  createFlat(const algebra::Vec3f &position, uint32_t u_patches,
             uint32_t v_patches);

  //  static std::shared_ptr<BezierSurfaceC2>
  // createCylinder(const algebra::Vec3f &position, float r, float h);

  void updateMesh() override {
    updateBezierSurface();
    _mesh = generateMesh();
  }
  void updateBezierSurface();
  static std::array<algebra::Vec3f, 16> convertDeBoorPatchToBernstein(
      const std::array<algebra::Vec3f, 16> &deBoorPatch);

private:
  explicit BezierSurfaceC2(const std::vector<algebra::Vec3f> &positions);
  std::vector<algebra::Vec3f> _bezierControlPoints;
  inline static int _id = 0;

  std::vector<algebra::Vec3f> getRepeatingDeBoorPoints();
  std::unique_ptr<BezierSurfaceMesh> generateMesh();
};