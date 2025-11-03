#pragma once

#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <memory>
#include <unistd.h>
#include <vector>

class BSplineCurve : public BezierCurve {
public:
  explicit BSplineCurve(
      const std::vector<std::reference_wrapper<PointEntity>> &points);

  bool acceptVisitor(IVisitor &visitor) override;
  void update() override;
  void updateBezier(const VirtualPoint &point, const algebra::Vec3f &pos);

  void addPoint(PointEntity &point) override;
  std::vector<VirtualPoint *> getVirtualPoints() const;
  bool &showBezierPoints();

private:
  inline static int kClassId;
  std::vector<std::unique_ptr<VirtualPoint>> _bezierPoints;
  bool _showBezierPoints = false;

  void recalculateBezierPoints();
  std::vector<std::unique_ptr<VirtualPoint>> initBezierPoints();
  std::unique_ptr<BezierMesh> generateMesh() override;
  void addBezierPoints();
};