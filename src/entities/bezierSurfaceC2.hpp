#pragma once
#include "bezierSurface.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <array>
#include <sys/types.h>
#include <vector>
class BezierSurfaceC2 : public BezierSurface {
public:
  explicit BezierSurfaceC2(
      const std::vector<std::reference_wrapper<PointEntity>> &points,
      uint32_t uCount, uint32_t vCount, algebra::ConnectionType connectionType);

  static std::vector<algebra::Vec3f>
  createFlatPositions(const algebra::Vec3f &position, uint32_t uPatches,
                      uint32_t vPatches, float uLength, float vLength);
  static std::vector<algebra::Vec3f>
  createCyllinderPositions(const algebra::Vec3f &position, uint32_t uPatches,
                           uint32_t vPatches, float r, float h);
  void updateMesh() override {
    updateBezierSurface();
    _mesh = generateMesh();
  }
  void updateBezierSurface();
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurfaceC2(*this);
  };
  static std::array<std::array<algebra::Vec3f, 4>, 4>
  processPatch(const std::array<std::array<algebra::Vec3f, 4>, 4> &patch);
  uint32_t getColCount() const override { return 3 + _patches.rowCount; }
  uint32_t getRowCount() const override { return 3 + _patches.colCount; }

private:
  std::vector<algebra::Vec3f> _bezierControlPoints;
  std::vector<algebra::Vec3f> _rowOrderBezierControlPoints;
  std::vector<algebra::Vec3f> getRowOrderedBezierPoints() const;
  void updateAlgebraicSurfaceC0() override;

  inline static int kClassId = 0;

  std::unique_ptr<BezierSurfaceMesh> generateMesh();
};