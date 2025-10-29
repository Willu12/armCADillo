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
  PointEntity *createPoint(const algebra::Vec3f &position);
  TorusEntity *createTorus(const algebra::Vec3f &position);

  std::optional<BezierCurveC0 *> createBezierCurveC0(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  std::optional<BSplineCurve *> createBSplineCurve(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  std::optional<InterpolatingSplineC2 *> createInterpolatingSpline(
      const std::vector<std::reference_wrapper<PointEntity>> &points);
  std::optional<BezierSurfaceC0 *>
  createBezierSurfaceC0(const algebra::Vec3f &position, uint32_t uPatches,
                        uint32_t vPatches, float u, float v, bool cyllinder);
  std::optional<BezierSurfaceC2 *>
  createBezierSurfaceC2(const algebra::Vec3f &position, uint32_t uPatches,
                        uint32_t vPatches, float u, float v, bool cyllinder);
  std::optional<GregorySurface *> createGregoryPatch(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces);

  std::optional<Polyline *>
  createPolyline(const std::vector<algebra::Vec3f> &points);

private:
  std::optional<std::unique_ptr<BezierSurfaceC0>>
  createFlatBezierSurfaceC0(const algebra::Vec3f &position, uint32_t uPatches,
                            uint32_t vPatches, float u, float v);
  std::optional<std::unique_ptr<BezierSurfaceC0>>
  createCyllinderBezierSurfaceC0(const algebra::Vec3f &position,
                                 uint32_t uPatches, uint32_t vPatches, float r,
                                 float h);
  std::optional<std::unique_ptr<BezierSurfaceC2>>
  createFlatBezierSurfaceC2(const algebra::Vec3f &position, uint32_t uPatches,
                            uint32_t vPatches, float u, float v);
  std::optional<std::unique_ptr<BezierSurfaceC2>>
  createCyllinderBezierSurfaceC2(const algebra::Vec3f &position,
                                 uint32_t uPatches, uint32_t vPatches, float r,
                                 float h);

  std::vector<std::reference_wrapper<PointEntity>>
  createSurfacePoints(const std::vector<algebra::Vec3f> &positions);
  Scene *scene_;
};