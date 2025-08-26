#include "entityFactory.hpp"
#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "entitiesTypes.hpp"
#include "interpolatingSplineC2.hpp"
#include "pointEntity.hpp"
#include "torusEntity.hpp"
#include <memory>

std::shared_ptr<PointEntity>
EntityFactory::createPoint(const algebra::Vec3f &position) {
  auto point = std::make_shared<PointEntity>(position);
  scene_->addEntity(EntityType::Point, point);
  return point;
}

std::shared_ptr<TorusEntity>
EntityFactory::createTorus(const algebra::Vec3f &position) {
  auto torus = std::make_shared<TorusEntity>(1.0, 0.3, position);
  scene_->addEntity(EntityType::Torus, torus);
  return torus;
}

std::optional<std::shared_ptr<BezierCurveC0>>
EntityFactory::createBezierCurveC0(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  if (points.size() < 2) {
    return std::nullopt;
  }

  auto bezierCurveC0 = std::make_shared<BezierCurveC0>(points);
  scene_->addEntity(EntityType::BezierCurveC0, bezierCurveC0);
  return bezierCurveC0;
}

std::optional<std::shared_ptr<BSplineCurve>> EntityFactory::createBSplineCurve(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  if (points.size() < 2) {
    return std::nullopt;
  }

  auto bSplineCurve = std::make_shared<BSplineCurve>(points);
  scene_->addEntity(EntityType::BSplineCurve, bSplineCurve);
  return bSplineCurve;
}

std::optional<std::shared_ptr<InterpolatingSplineC2>>
EntityFactory::createInterpolatingSpline(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  if (points.size() < 2) {
    return std::nullopt;
  }

  auto spline = std::make_shared<InterpolatingSplineC2>(points);
  scene_->addEntity(EntityType::InterpolatingSplineCurve, spline);
  return spline;
}