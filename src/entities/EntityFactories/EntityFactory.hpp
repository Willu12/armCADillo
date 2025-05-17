#pragma once

#include "IEntity.hpp"
#include "bezierCurveC0.hpp"
#include "pointEntity.hpp"
#include <functional>
class EntityFactory {
  std::shared_ptr<IEntity> createPoint(const algebra::Vec3f &position) {
    return std::make_shared<PointEntity>(position);
  }
  std::shared_ptr<IEntity> createTorus(const algebra::Vec3f &position) {
    return std::make_shared<PointEntity>(1.f, .3f, position);
  }
  std::shared_ptr<IEntity> createBezierCurve(
      const std::vector<std::reference_wrapper<PointEntity>> &points) {
    return std::make_shared<BezierCurveC0>(points);
  }
  std::shared_ptr<IEntity> createBSplineCurve();
  std::shared_ptr<IEntity> createInterpolatingSplineCurve();
  std::shared_ptr<IEntity> createBezierSurfaceC0();
};