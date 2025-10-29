#include "entityFactory.hpp"
#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurfaceC0.hpp"
#include "entitiesTypes.hpp"
#include "interpolatingSplineC2.hpp"
#include "pointEntity.hpp"
#include "polyline.hpp"
#include "torusEntity.hpp"
#include <functional>
#include <memory>

PointEntity *EntityFactory::createPoint(const algebra::Vec3f &position) {
  auto point = std::make_unique<PointEntity>(position);
  auto point_ptr = point.get();
  scene_->addEntity(EntityType::Point, std::move(point));
  return point_ptr;
}

TorusEntity *EntityFactory::createTorus(const algebra::Vec3f &position) {
  auto torus = std::make_unique<TorusEntity>(1.0, 0.3, position);
  auto torus_ptr = torus.get();
  scene_->addEntity(EntityType::Torus, std::move(torus));
  return torus_ptr;
}

std::optional<BezierCurveC0 *> EntityFactory::createBezierCurveC0(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  if (points.size() < 2) {
    return std::nullopt;
  }

  auto bezier_c0 = std::make_unique<BezierCurveC0>(points);
  auto *bezier_c0_ptr = bezier_c0.get();
  scene_->addEntity(EntityType::BezierCurveC0, std::move(bezier_c0));
  return bezier_c0_ptr;
}

std::optional<BSplineCurve *> EntityFactory::createBSplineCurve(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  if (points.size() < 2) {
    return std::nullopt;
  }

  auto b_spline = std::make_unique<BSplineCurve>(points);
  auto *b_spline_ptr = b_spline.get();
  scene_->addEntity(EntityType::BSplineCurve, std::move(b_spline));
  return b_spline_ptr;
}

std::optional<InterpolatingSplineC2 *> EntityFactory::createInterpolatingSpline(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  if (points.size() < 2) {
    return std::nullopt;
  }

  auto spline = std::make_unique<InterpolatingSplineC2>(points);
  auto *spline_ptr = spline.get();
  scene_->addEntity(EntityType::InterpolatingSplineCurve, std::move(spline));
  return spline_ptr;
}

std::optional<BezierSurfaceC0 *>
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
  auto *entity_ptr = entity->get();
  scene_->addEntity(EntityType::BezierSurfaceC0, std::move(*entity));
  return entity_ptr;
}
std::optional<BezierSurfaceC2 *>
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

  auto *entity_ptr = entity->get();
  scene_->addEntity(EntityType::BezierSurfaceC2, std::move(*entity));
  return entity_ptr;
}

std::optional<std::unique_ptr<BezierSurfaceC0>>
EntityFactory::createFlatBezierSurfaceC0(const algebra::Vec3f &position,
                                         uint32_t uPatches, uint32_t vPatches,
                                         float u, float v) {
  const auto &positions =
      BezierSurfaceC0::createFlatPositions(position, uPatches, vPatches, u, v);
  const auto &points = createSurfacePoints(positions);
  return std::make_unique<BezierSurfaceC0>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Flat);
}
std::optional<std::unique_ptr<BezierSurfaceC0>>
EntityFactory::createCyllinderBezierSurfaceC0(const algebra::Vec3f &position,
                                              uint32_t uPatches,
                                              uint32_t vPatches, float r,
                                              float h) {
  const auto &positions = BezierSurfaceC0::createCyllinderPositions(
      position, uPatches, vPatches, r, h);
  auto points = createSurfacePoints(positions);
  for (int i = 0; i < (3 * uPatches + 1); ++i) {
    points.push_back(points[i]);
  }
  return std::make_unique<BezierSurfaceC0>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Columns);
}
std::optional<std::unique_ptr<BezierSurfaceC2>>
EntityFactory::createFlatBezierSurfaceC2(const algebra::Vec3f &position,
                                         uint32_t uPatches, uint32_t vPatches,
                                         float u, float v) {
  const auto &positions =
      BezierSurfaceC2::createFlatPositions(position, uPatches, vPatches, u, v);
  auto points = createSurfacePoints(positions);

  return std::make_unique<BezierSurfaceC2>(points, uPatches, vPatches,
                                           algebra::ConnectionType::Flat);
}
std::optional<std::unique_ptr<BezierSurfaceC2>>
EntityFactory::createCyllinderBezierSurfaceC2(const algebra::Vec3f &position,
                                              uint32_t uPatches,
                                              uint32_t vPatches, float r,
                                              float h) {
  const auto &positions = BezierSurfaceC2::createCyllinderPositions(
      position, uPatches, vPatches, r, h);
  auto points = createSurfacePoints(positions);

  const uint32_t uPoints = 3 + uPatches;
  const uint32_t vPoints = 3 + vPatches;
  std::vector<std::reference_wrapper<PointEntity>> fixedPoints;
  fixedPoints.reserve(vPoints * uPoints);
  for (uint32_t row = 0; row < vPoints; ++row) {
    for (uint32_t col = 0; col < uPoints - 3; ++col) {
      fixedPoints.push_back(points[row * (uPoints - 3) + col]);
    }
    for (uint32_t col = 0; col < 3; ++col) {
      fixedPoints.push_back(points[row * (uPoints - 3) + col]);
    }
  }
  return std::make_unique<BezierSurfaceC2>(fixedPoints, uPatches, vPatches,
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

std::optional<GregorySurface *> EntityFactory::createGregoryPatch(
    const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {
  auto gregory_surfaces = GregorySurface::createGregorySurfaces(surfaces);
  auto *gregory_surface_ptr = gregory_surfaces[0].get();

  for (auto &gregorySurface : gregory_surfaces) {
    scene_->addEntity(EntityType::GregorySurface, std::move(gregorySurface));
  }
  return gregory_surface_ptr;
}

std::optional<Polyline *>
EntityFactory::createPolyline(const std::vector<algebra::Vec3f> &points) {
  auto polyline = std::make_unique<Polyline>(points);
  auto polyline_ptr = polyline.get();
  scene_->addEntity(EntityType::Polyline, std::move(polyline));
  return polyline_ptr;
}