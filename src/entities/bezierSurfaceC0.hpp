#pragma once
#include "IDifferentialParametricForm.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceMesh.hpp"
#include "pointEntity.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <array>
#include <functional>
#include <memory>
#include <vector>

class BezierSurfaceC0 : public BezierSurface,
                        public algebra::IDifferentialParametricForm<2, 3> {
public:
  explicit BezierSurfaceC0(
      const std::vector<std::reference_wrapper<PointEntity>> &points,
      uint32_t uCount, uint32_t vCount, bool cyllinder);

  static std::vector<algebra::Vec3f>
  createFlatPositions(const algebra::Vec3f &position, uint32_t uPatches,
                      uint32_t vPatches, float uLength, float vLength);
  static std::vector<algebra::Vec3f>
  createCyllinderPositions(const algebra::Vec3f &position, uint32_t uPatches,
                           uint32_t vPatches, float r, float h);

  void updateMesh() override {
    updateAlgebraSurface();
    _mesh = generateMesh();
  }
  uint32_t getColCount() const override { return 3 * _patches.colCount + 1; }
  uint32_t getRowCount() const override { return 3 * _patches.rowCount + 1; }
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurfaceC0(*this);
  };

  bool wrapped(size_t dim) const override;
  std::array<algebra::Vec2f, 2> bounds() const override;
  algebra::Vec3f value(const algebra::Vec2f &pos) const override;
  std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const override;
  algebra::Matrix<float, 3, 2>
  jacobian(const algebra::Vec2f &pos) const override;

private:
  inline static int kClassId = 0;
  std::unique_ptr<algebra::BezierSurfaceC0> _algebraSurface;

  std::unique_ptr<BezierSurfaceMesh> generateMesh();
  algebra::BezierSurfaceC0 getBezierSurface() const;
  void updateAlgebraSurface();
};
