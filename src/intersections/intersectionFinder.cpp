#include "intersectionFinder.hpp"
#include "functions.hpp"
#include "gradientDescent.hpp"
#include "newtonMethod.hpp"
#include "vec.hpp"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <optional>
#include <print>
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

std::optional<Intersection> IntersectionFinder::find(bool same) const {
  std::optional<IntersectionPoint> firstPoint = findFirstPoint(same);
  if (!firstPoint) {
    return std::nullopt;
  }

  auto nextPoints = findNextPoints(*firstPoint, false);
  if (nextPoints && nextPoints->looped) {
    return nextPoints;
  }

  auto previousPoints = findNextPoints(*firstPoint, true);

  return connectFoundPoints(nextPoints, previousPoints);
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointStochastic() const {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_.lock()->bounds()[0][0],
                                             surface0_.lock()->bounds()[0][1]);
  for (std::size_t stochTry = 0; stochTry <= kStochasticTries; ++stochTry) {
    if (stochTry > 0 && stochTry % 100 == 0) {
      std::println("stochastic try {}", stochTry);
    }
    const auto point0 = algebra::Vec2f(dist(gen), dist(gen));
    const auto point0Value = surface0_.lock()->value(point0);

    const auto point1 = findPointProjection(surface1_, point0Value);

    if (auto intersectionPoint = findCommonSurfacePoint(point0, point1)) {
      return intersectionPoint;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointSameStochastic() const {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_.lock()->bounds()[0][0],
                                             surface0_.lock()->bounds()[0][1]);
  for (std::size_t stochTry = 0; stochTry < kStochasticTries; ++stochTry) {
    const auto point0 = algebra::Vec2f(dist(gen), dist(gen));
    const auto point1 = algebra::Vec2f(dist(gen), dist(gen));

    if (auto intersectionPoint = findCommonSurfacePoint(point0, point1)) {
      return intersectionPoint;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointWithGuidance() const {
  for (std::size_t stochTry = 0; stochTry < kStochasticTries; ++stochTry) {
    if (stochTry > 0 && stochTry % 100 == 0) {
      std::println("stochastic try {}", stochTry);
    }
    auto point0 = findPointProjection(surface0_, *guidancePoint_);
    auto point1 = findPointProjection(surface1_, *guidancePoint_);
    if (auto intersectionPoint = findCommonSurfacePoint(point0, point1)) {
      return intersectionPoint;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointSameWithGuidance() const {
  auto point0 = findPointProjection(surface0_, *guidancePoint_);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_.lock()->bounds()[0][0],
                                             surface0_.lock()->bounds()[0][1]);
  for (std::size_t stochTry = 0; stochTry < kStochasticTries; ++stochTry) {
    const auto point0Value = surface0_.lock()->value(point0);
    const auto point1 = findPointProjection(surface0_, point0Value);

    if (auto intersectionPoint = findCommonSurfacePoint(point0, point1)) {
      return intersectionPoint;
    }
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

  if ((surface0Val - surface1Val).length() > 10e-2) {
    std::println("Failed to find max prec = {}\n",
                 (surface0Val - surface1Val).length());
    return std::nullopt;
  }

  std::println("found first approximation of dist == {}",
               (surface0Val - surface1Val).length());

  auto firstIntersection =
      IntersectionPoint{.surface0 = surface0Minimum,
                        .surface1 = surface1Minimum,
                        .point = (surface0Val + surface1Val) / 2.f};

  // return firstIntersection;
  return newtowRefinment(firstIntersection);
}

std::optional<IntersectionPoint>
IntersectionFinder::newtowRefinment(const IntersectionPoint &point) const {
  auto function =
      std::make_unique<algebra::IntersectionFunction>(surface0_, surface1_);

  algebra::Vec4f startingPoint{point.surface0[0], point.surface0[1],
                               point.surface1[0], point.surface1[1]};
  algebra::NewtonMethod<4, 4> newton(std::move(function), startingPoint);
  // newton.setIterationCount(2);

  auto newtonResult = newton.calculate();
  if (!newtonResult) {
    std::println("Newton failed to calculate\n");
    return std::nullopt;
  }
  auto minimum = *newtonResult;
  auto surface0Minimum = algebra::Vec2f{minimum[0], minimum[1]};
  auto surface1Minimum = algebra::Vec2f{minimum[2], minimum[3]};

  auto surface0Val = surface0_.lock()->value(surface0Minimum);
  auto surface1Val = surface1_.lock()->value(surface1Minimum);

  if ((surface0Val - surface1Val).length() > 10e-3) {
    std::println("Newton Refinment fail maxPreccsion == {}",
                 (surface0Val - surface1Val).length());
    return std::nullopt;
  }

  std::println("found Newton Refinment of size  approximation of dist == {}",
               (surface0Val - surface1Val).length());

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

    if (i > 0 && i % 300 == 0) {
      std::println("Newton found first {} points", i);
    }
    if (i > 2 && intersectionLooped(Intersection{.points = points})) {
      std::println("Found loop ending Newton");
      points.back() = points.front();
      return Intersection{.points = points, .looped = true};
    }
    if (nextPoint) {
      points.push_back(*nextPoint);
    } else {
      std::println("Newton method failed to find next point on {} iteration",
                   i);
      break;
    }
  }

  return Intersection{.points = points, .firstPoint = firstPoint.point};
};

std::optional<IntersectionPoint>
IntersectionFinder::nextIntersectionPoint(const IntersectionPoint &lastPoint,
                                          bool reversed) const {

  auto tangent = getTangent(lastPoint);
  if (reversed) {
    tangent = tangent * -1.f;
  }

  auto function = std::make_unique<algebra::IntersectionStepFunction>(
      surface0_, surface1_, lastPoint.point, tangent);
  function->setStep(config_.intersectionStep_);
  algebra::NewtonMethod<4, 4> newton(
      std::move(function),
      algebra::Vec4f(lastPoint.surface0[0], lastPoint.surface0[1],
                     lastPoint.surface1[0], lastPoint.surface1[1]));
  const auto nextIntersection = newton.calculate();

  if (nextIntersection) {
    auto minimum = *nextIntersection;
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

  // fixIntersectionPointsEdges(points);

  if (points.size() == 0) {
    return std::nullopt;
  }

  return Intersection{.points = points,
                      .firstPoint = nextPoints->points[0].point};
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPoint(bool same) const {
  if (same) {
    return config_.useCursor_ ? findFirstPointSameWithGuidance()
                              : findFirstPointSameStochastic();
  }
  return config_.useCursor_ ? findFirstPointWithGuidance()
                            : findFirstPointStochastic();
}

void IntersectionFinder::fixIntersectionPointsEdges(
    std::vector<IntersectionPoint> &points) const {
  if (points.empty()) {
    return;
  }

  auto snap = [](float &value, const algebra::Vec2f &min,
                 const algebra::Vec2f &max, int dim) {
    float epsilon = 0.001f;
    if (std::abs(value - min[dim]) < epsilon) {
      value = min[dim];
    } else if (std::abs(value - max[dim]) < epsilon) {
      value = max[dim];
    }
  };

  const auto surface0 = surface0_.lock();
  const auto surface1 = surface1_.lock();
  if (!surface0 || !surface1) {
    return;
  }

  const auto bounds0 = surface0->bounds();
  const auto bounds1 = surface1->bounds();

  for (int i : {0, static_cast<int>(points.size()) - 1}) {
    snap(points[i].surface0[0], bounds0[0], bounds0[1], 0);
    snap(points[i].surface0[1], bounds0[0], bounds0[1], 1);
    snap(points[i].surface1[0], bounds1[0], bounds1[1], 0);
    snap(points[i].surface1[1], bounds1[0], bounds1[1], 1);
  }
}

bool IntersectionFinder::intersectionLooped(
    const Intersection &intersection) const {
  const auto &firstPoint = intersection.points.front();
  const auto &lastPoint = intersection.points.back();
  auto surf0Wrapped =
      surface0_.lock()->wrapped(0) || surface0_.lock()->wrapped(1);
  auto surf1Wrapped =
      surface1_.lock()->wrapped(0) || surface1_.lock()->wrapped(1);

  const auto dist = 0.005f;
  return ((firstPoint.surface0 - lastPoint.surface0).length() < dist &&
          surf0Wrapped) &&
         ((firstPoint.surface1 - lastPoint.surface1).length() < dist &&
          surf1Wrapped);
}