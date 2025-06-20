#pragma once
#include "IDifferentialParametricForm.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <memory>
#include <vector>

class BezierSurfaceC0 : public BezierSurface,
                        public IDifferentialParametricForm {
public:
  explicit BezierSurfaceC0(
      const std::vector<std::reference_wrapper<PointEntity>> &points,
      uint32_t uCount, uint32_t vCount);

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

  algebra::Vec2f bounds() const override;
  algebra::Vec3f value(const algebra::Vec2f &pos) const override;
  std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const override;
  float bernstein(int i, int n, float t) const;

private:
  inline static int kClassId = 0;
  std::unique_ptr<BezierSurfaceMesh> generateMesh();
};
