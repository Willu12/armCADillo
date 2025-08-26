#pragma once

#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "interpolatingSplineC2.hpp"
#include "pointEntity.hpp"
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

private:
  Scene *scene_;
};