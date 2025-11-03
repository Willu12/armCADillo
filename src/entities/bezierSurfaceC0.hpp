#pragma once
#include "bezierSurface.hpp"
#include "bezierSurfaceMesh.hpp"
#include "pointEntity.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <functional>
#include <memory>
#include <vector>

class BezierSurfaceC0 : public BezierSurface {
public:
  explicit BezierSurfaceC0(
      const std::vector<std::reference_wrapper<PointEntity>> &points,
      uint32_t uCount, uint32_t vCount, algebra::ConnectionType connectionType);

  static std::vector<algebra::Vec3f>
  createFlatPositions(const algebra::Vec3f &position, uint32_t uPatches,
                      uint32_t vPatches, float uLength, float vLength);
  static std::vector<algebra::Vec3f>
  createCyllinderPositions(const algebra::Vec3f &position, uint32_t uPatches,
                           uint32_t vPatches, float r, float h);

  void updateMesh() override { _mesh = generateMesh(); }
  uint32_t getColCount() const override { return 3 * _patches.colCount + 1; }
  uint32_t getRowCount() const override { return 3 * _patches.rowCount + 1; }
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurfaceC0(*this);
  };

private:
  inline static int kClassId = 0;
  std::unique_ptr<BezierSurfaceMesh> generateMesh();
  void updateAlgebraicSurfaceC0() override;
};
