#pragma once

#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurfaceC0.hpp"
#include "bezierSurfaceC2.hpp"
#include "gregorySurface.hpp"
#include "interpolatingSplineC2.hpp"
#include "pointEntity.hpp"
#include "polyline.hpp"
#include "scene.hpp"
#include "torusEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <memory>
#include <optional>
class EntityFactory {
public:
  explicit EntityFactory(Scene *scene) : scene_(scene) {}
  std::shared_ptr<PointEntity> createPoint(const algebra::Vec3f &position);
  std::shared_ptr<TorusEntity> createTorus(const algebra::Vec3f &position);

  std::optional<std::shared_ptr<BezierCurveC0>> createBezierCurveC0(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  std::optional<std::shared_ptr<BSplineCurve>> createBSplineCurve(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  std::optional<std::shared_ptr<InterpolatingSplineC2>>
  createInterpolatingSpline(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  std::optional<std::shared_ptr<BezierSurfaceC0>>
  createBezierSurfaceC0(const algebra::Vec3f &position, uint32_t uPatches,
                        uint32_t vPatches, float u, float v, bool cyllinder);
  std::optional<std::shared_ptr<BezierSurfaceC2>>
  createBezierSurfaceC2(const algebra::Vec3f &position, uint32_t uPatches,
                        uint32_t vPatches, float u, float v, bool cyllinder);
  std::optional<std::shared_ptr<GregorySurface>> createGregoryPatch(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);

  std::optional<std::shared_ptr<Polyline>>
  createPolyline(const std::vector<algebra::Vec3f> &points);

private:
  std::optional<std::shared_ptr<BezierSurfaceC0>>
  createFlatBezierSurfaceC0(const algebra::Vec3f &position, uint32_t uPatches,
                            uint32_t vPatches, float u, float v);
  std::optional<std::shared_ptr<BezierSurfaceC0>>
  createCyllinderBezierSurfaceC0(const algebra::Vec3f &position,
                                 uint32_t uPatches, uint32_t vPatches, float r,
                                 float h);
  std::optional<std::shared_ptr<BezierSurfaceC2>>
  createFlatBezierSurfaceC2(const algebra::Vec3f &position, uint32_t uPatches,
                            uint32_t vPatches, float u, float v);
  std::optional<std::shared_ptr<BezierSurfaceC2>>
  createCyllinderBezierSurfaceC2(const algebra::Vec3f &position,
                                 uint32_t uPatches, uint32_t vPatches, float r,
                                 float h);

  std::vector<std::reference_wrapper<PointEntity>>
  createSurfacePoints(const std::vector<algebra::Vec3f> &positions);
  Scene *scene_;
};