#include "entityFactory.hpp"
#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurfaceC0.hpp"
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

std::optional<std::shared_ptr<BezierSurfaceC0>>
EntityFactory::createBezierSurfaceC0(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float u, float v, bool cyllinder) {
  auto entity =
      cyllinder
          ? createCyllinderBezierSurfaceC0(position, uPatches, vPatches, u, v)
          : createFlatBezierSurfaceC0(position, uPatches, vPatches, u, v);

  if (!entity) {
    return std::nullopt;
  }

  scene_->addEntity(EntityType::BezierSurfaceC0, *entity);
  return entity;
}
std::optional<std::shared_ptr<BezierSurfaceC2>>
EntityFactory::createBezierSurfaceC2(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float u, float v, bool cyllinder) {
  auto entity =
      cyllinder
          ? createCyllinderBezierSurfaceC2(position, uPatches, vPatches, u, v)
          : createFlatBezierSurfaceC2(position, uPatches, vPatches, u, v);

  if (!entity) {
    return std::nullopt;
  }

  scene_->addEntity(EntityType::BezierSurfaceC2, *entity);
  return entity;
}

std::optional<std::shared_ptr<BezierSurfaceC0>>
EntityFactory::createFlatBezierSurfaceC0(const algebra::Vec3f &position,
                                         uint32_t uPatches, uint32_t vPatches,
                                         float u, float v) {
  const auto &positions =
      BezierSurfaceC0::createFlatPositions(position, uPatches, vPatches, u, v);
  const auto &points = createSurfacePoints(positions);
  return std::make_shared<BezierSurfaceC0>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Flat);
}
std::optional<std::shared_ptr<BezierSurfaceC0>>
EntityFactory::createCyllinderBezierSurfaceC0(const algebra::Vec3f &position,
                                              uint32_t uPatches,
                                              uint32_t vPatches, float r,
                                              float h) {
  const auto &positions = BezierSurfaceC0::createCyllinderPositions(
      position, uPatches, vPatches, r, h);
  const auto &points = createSurfacePoints(positions);
  return std::make_shared<BezierSurfaceC0>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Columns);
}
std::optional<std::shared_ptr<BezierSurfaceC2>>
EntityFactory::createFlatBezierSurfaceC2(const algebra::Vec3f &position,
                                         uint32_t uPatches, uint32_t vPatches,
                                         float u, float v) {
  const auto &positions =
      BezierSurfaceC2::createFlatPositions(position, uPatches, vPatches, u, v);
  const auto &points = createSurfacePoints(positions);
  return std::make_shared<BezierSurfaceC2>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Flat);
}
std::optional<std::shared_ptr<BezierSurfaceC2>>
EntityFactory::createCyllinderBezierSurfaceC2(const algebra::Vec3f &position,
                                              uint32_t uPatches,
                                              uint32_t vPatches, float r,
                                              float h) {
  const auto &positions = BezierSurfaceC2::createCyllinderPositions(
      position, uPatches, vPatches, r, h);
  const auto &points = createSurfacePoints(positions);
  return std::make_shared<BezierSurfaceC2>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Columns);
}

std::vector<std::reference_wrapper<PointEntity>>
EntityFactory::createSurfacePoints(
    const std::vector<algebra::Vec3f> &positions) {
  std::vector<std::reference_wrapper<PointEntity>> points;
  points.reserve(positions.size());

  for (const auto &pos : positions) {
    auto point = createPoint(pos);
    points.emplace_back(*point);
  }

  return points;
}