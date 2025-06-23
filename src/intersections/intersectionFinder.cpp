#include "intersectionFinder.hpp"
#include "functions.hpp"
#include "gradientDescent.hpp"
#include "newtonMethod.hpp"
#include "vec.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <random>
#include <vector>

using algebra::Vec3f;

void IntersectionFinder::setSurfaces(
    std::shared_ptr<algebra::IDifferentialParametricForm<2, 3>> surface0,
    std::shared_ptr<algebra::IDifferentialParametricForm<2, 3>> surface1) {
  surface0_ = surface0;
  surface1_ = surface1;
}

void IntersectionFinder::setGuidancePoint(const algebra::Vec3f &guidancePoint) {
  guidancePoint_ = guidancePoint;
}

std::optional<Intersection> IntersectionFinder::find() const {
  const auto firstPoint = findFirstPoint();
  if (!firstPoint)
    return std::nullopt;

  auto nextPoints = findNextPoints(*firstPoint, false);
  auto previousPoints = findNextPoints(*firstPoint, true);

  return connectFoundPoints(nextPoints, previousPoints);
}

std::optional<IntersectionPoint> IntersectionFinder::findFirstPoint() const {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_.lock()->bounds()[0][0],
                                             surface0_.lock()->bounds()[0][1]);
  for (std::size_t stochTry = 0; stochTry < kStochasticTries; ++stochTry) {
    const auto point0 = algebra::Vec2f(dist(gen), dist(gen));
    const auto point0Value = surface0_.lock()->value(point0);

    const auto point1 = findPointProjection(surface1_, point0Value);

    if (auto intersectionPoint = findCommonSurfacePoint(point0, point1))
      return *intersectionPoint;
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findCommonSurfacePoint(const algebra::Vec2f &start0,
                                           const algebra::Vec2f &start1) const {
  auto function = std::make_unique<algebra::SurfaceSurfaceL2DistanceSquared>(
      surface0_, surface1_);
  algebra::GradientDescent<4> gradientDescent(std::move(function));

  gradientDescent.setLearningRate(config_.numericalStep_);
  gradientDescent.setStartingPoint(
      algebra::Vec4f(start0[0], start0[1], start1[0], start1[1]));

  auto minimum = gradientDescent.calculate();
  auto surface0Minimum = algebra::Vec2f{minimum[0], minimum[1]};
  auto surface1Minimum = algebra::Vec2f{minimum[2], minimum[3]};

  auto surface0Val = surface0_.lock()->value(surface0Minimum);
  auto surface1Val = surface1_.lock()->value(surface1Minimum);

  if ((surface0Val - surface1Val).length() > 0.001f)
    return std::nullopt;

  return IntersectionPoint{.surface0 = surface0Minimum,
                           .surface1 = surface1Minimum,
                           .point = (surface0Val + surface1Val) / 2.f};
}

algebra::Vec2f IntersectionFinder::findPointProjection(
    std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface,
    algebra::Vec3f surfacePoint) const {
  auto function = std::make_unique<algebra::SurfacePointL2DistanceSquared>(
      surface, surfacePoint);

  algebra::GradientDescent<2> gradientDescent(std::move(function));

  return gradientDescent.calculate();
}

algebra::Vec3f
IntersectionFinder::getTangent(const IntersectionPoint &firstPoint) const {
  auto [du0, dv0] = surface0_.lock()->derivatives(firstPoint.surface0);
  auto [du1, dv1] = surface1_.lock()->derivatives(firstPoint.surface1);

  Vec3f n0 = du0.cross(dv0);
  Vec3f n1 = du1.cross(dv1);

  Vec3f tangent = n0.cross(n1);

  return tangent.normalize();
}

std::optional<Intersection>
IntersectionFinder::findNextPoints(const IntersectionPoint &firstPoint,
                                   bool reversed) const {
  std::vector<IntersectionPoint> points;
  points.push_back(firstPoint);
  for (std::size_t i = 0; i < kMaxIntersectionCurvePoint; ++i) {
    auto nextPoint = nextIntersectionPoint(points.back(), reversed);
    if (nextPoint)
      points.push_back(*nextPoint);
  }

  return Intersection{.points = points};
};

std::optional<IntersectionPoint>
IntersectionFinder::nextIntersectionPoint(const IntersectionPoint &lastPoint,
                                          bool reversed) const {

  auto tangent = getTangent(lastPoint);
  if (reversed)
    tangent = tangent * -1.f;

  auto function = std::make_unique<algebra::IntersectionStepFunction>(
      surface0_, surface1_, lastPoint.point, tangent);
  function->setStep(config_.intersectionStep_);
  algebra::NewtonMethod newton(
      std::move(function),
      algebra::Vec4f(lastPoint.surface0[0], lastPoint.surface0[1],
                     lastPoint.surface1[0], lastPoint.surface1[1]));
  const auto correctedIntersection = newton.calculate();

  if (correctedIntersection) {
    auto minimum = *correctedIntersection;
    auto surface0Minimum = algebra::Vec2f{minimum[0], minimum[1]};
    auto surface1Minimum = algebra::Vec2f{minimum[2], minimum[3]};

    auto surface0Val = surface0_.lock()->value(surface0Minimum);
    auto surface1Val = surface1_.lock()->value(surface1Minimum);

    return IntersectionPoint{.surface0 = surface0Minimum,
                             .surface1 = surface1Minimum,
                             .point = (surface0Val + surface1Val) / 2.f};
  }

  return std::nullopt;
};

std::optional<Intersection> IntersectionFinder::connectFoundPoints(
    const std::optional<Intersection> &nextPoints,
    const std::optional<Intersection> &previousPoints) const {

  std::vector<IntersectionPoint> points;
  if (previousPoints) {
    points.insert(points.end(), previousPoints->points.begin(),
                  previousPoints->points.end());
    std::ranges::reverse(points);
  }
  if (nextPoints) {
    points.insert(points.end(), nextPoints->points.begin(),
                  nextPoints->points.end());
  }

  if (points.size() == 0)
    return std::nullopt;

  return Intersection{.points = points};
}