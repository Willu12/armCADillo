#include "intersectionFinder.hpp"
#include "distribution.hpp"
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

static constexpr float kProjectionTolerance = 0.5f;
static constexpr std::pair<int, int> kGridSearchResMinMax = {200, 300};

using algebra::Vec3f;

void IntersectionFinder::setSurfaces(
    const algebra::IDifferentialParametricForm<2, 3> *surface0,
    const algebra::IDifferentialParametricForm<2, 3> *surface1) {
  surface0_ = surface0;
  surface1_ = surface1;
}

void IntersectionFinder::setGuidancePoint(const algebra::Vec3f &guidancePoint) {
  guidancePoint_ = guidancePoint;
}

std::optional<Intersection> IntersectionFinder::find(bool same) const {
  std::optional<IntersectionPoint> first_point = findFirstPoint(same);
  std::println("Starts looking for first point!");
  if (!first_point) {
    std::println("failed to find first point!");
    return std::nullopt;
  }

  std::println("Found first point!");

  auto next_points = findNextPoints(*first_point, false);
  if (next_points && next_points->looped) {
    return next_points;
  }

  auto previous_points = findNextPoints(*first_point, true);
  return connectFoundPoints(next_points, previous_points);
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointStochastic() const {
  std::println("Find first stochastic start");
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_->bounds()[0][0],
                                             surface0_->bounds()[0][1]);

  for (std::size_t stoch_try = 0; stoch_try <= kStochasticTries; ++stoch_try) {
    if (stoch_try > 0 && stoch_try % 100 == 0) {
      std::println("stochastic try {}", stoch_try);
    }
    const auto point0 = algebra::Vec2f(dist(gen), dist(gen));
    const auto point_0_value = surface0_->value(point0);

    const auto point1 = findPointProjection(surface1_, point_0_value);
    if (!point1) {
      std::println("Couldn't find poitn projection");
      continue;
    }

    std::println("Found point projection");
    if (auto intersection_point = findCommonSurfacePoint(point0, *point1)) {
      return intersection_point;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointSameStochastic() const {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_->bounds()[0][0],
                                             surface0_->bounds()[0][1]);
  for (std::size_t stoch_try = 0; stoch_try < kStochasticTries; ++stoch_try) {
    const auto point0 = algebra::Vec2f(dist(gen), dist(gen));
    const auto point1 = algebra::Vec2f(dist(gen), dist(gen));

    if (auto intersection_point = findCommonSurfacePoint(point0, point1)) {
      return intersection_point;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointWithGuidance() const {
  for (std::size_t stoch_try = 0; stoch_try < kStochasticTries; ++stoch_try) {
    if (stoch_try > 0 && stoch_try % 10 == 0) {
      config_.numericalStep_ *= 2.f;
    }

    auto point0 = findPointProjection(surface0_, *guidancePoint_);
    if (!point0) {
      continue;
    }

    auto point1 = findPointProjection(surface1_, *guidancePoint_);
    if (!point1) {
      continue;
    }

    if (auto intersection_point = findCommonSurfacePoint(*point0, *point1)) {
      return intersection_point;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findFirstPointSameWithGuidance() const {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(surface0_->bounds()[0][0],
                                             surface0_->bounds()[0][1]);
  for (std::size_t stoch_try = 0; stoch_try < kStochasticTries; ++stoch_try) {
    auto point0 = findPointProjection(surface0_, *guidancePoint_);
    if (!point0) {
      continue;
    }

    const auto point1 =
        findPointProjection(surface0_, surface0_->value(*point0));

    if (!point1) {
      continue;
    }

    if (auto intersection_point = findCommonSurfacePoint(*point0, *point1)) {
      return intersection_point;
    }
  }
  return std::nullopt;
}

std::optional<IntersectionPoint>
IntersectionFinder::findCommonSurfacePoint(const algebra::Vec2f &start0,
                                           const algebra::Vec2f &start1) const {
  auto function = std::make_unique<algebra::SurfaceSurfaceL2DistanceSquared>(
      surface0_, surface1_);
  algebra::GradientDescent<4> gradient_descent(std::move(function));

  gradient_descent.setLearningRate(config_.numericalStep_);
  gradient_descent.setStartingPoint(
      algebra::Vec4f(start0[0], start0[1], start1[0], start1[1]));

  auto gradient_result = gradient_descent.calculate();
  if (!gradient_result) {
    return std::nullopt;
  }
  auto minimum = *gradient_result;
  auto surface_0_minimum = algebra::Vec2f{minimum[0], minimum[1]};
  auto surface_1_minimum = algebra::Vec2f{minimum[2], minimum[3]};

  auto surface_0_val = surface0_->value(surface_0_minimum);
  auto surface_1_val = surface1_->value(surface_1_minimum);

  if ((surface_0_val - surface_1_val).length() > 0.1) {
    std::println("Failed to find max prec = {}\n",
                 (surface_0_val - surface_1_val).length());
    return std::nullopt;
  }

  auto first_intersection =
      IntersectionPoint{.surface0 = surface_0_minimum,
                        .surface1 = surface_1_minimum,
                        .point = (surface_0_val + surface_1_val) / 2.f};

  return newtowRefinment(first_intersection);
}

std::optional<IntersectionPoint>
IntersectionFinder::newtowRefinment(const IntersectionPoint &point) const {
  auto function =
      std::make_unique<algebra::IntersectionFunction>(surface0_, surface1_);

  algebra::Vec4f starting_point{point.surface0[0], point.surface0[1],
                                point.surface1[0], point.surface1[1]};
  algebra::NewtonMethod<4, 4> newton(std::move(function), starting_point);
  // newton.setIterationCount(2);

  auto newton_result = newton.calculate();
  if (!newton_result) {
    return std::nullopt;
  }

  auto minimum = *newton_result;
  auto surface_0_minimum = algebra::Vec2f{minimum[0], minimum[1]};
  auto surface_1_minimum = algebra::Vec2f{minimum[2], minimum[3]};

  auto surface_0_val = surface0_->value(surface_0_minimum);
  auto surface_1_val = surface1_->value(surface_1_minimum);

  if ((surface_0_val - surface_1_val).length() > 10e-3) {
    std::println("Newton Refinment fail maxPreccsion == {}",
                 (surface_0_val - surface_1_val).length());
    return std::nullopt;
  }

  return IntersectionPoint{.surface0 = surface_0_minimum,
                           .surface1 = surface_1_minimum,
                           .point = (surface_0_val + surface_1_val) / 2.f};
}

std::optional<algebra::Vec2f> IntersectionFinder::findPointProjection(
    const algebra::IDifferentialParametricForm<2, 3> *surface,
    algebra::Vec3f surfacePoint) const {
  for (std::size_t i = 0; i < 1; ++i) {
    if (i > 0 && i % 100 == 0) {
      std::println("point projection try {}", i);
    }
    auto guess = findInitialGuessWithGuidance(
        surface, surfacePoint,
        algebra::Distribution::randomInt(kGridSearchResMinMax.first,
                                         kGridSearchResMinMax.second));

    auto function = std::make_unique<algebra::SurfacePointL2DistanceSquared>(
        surface, surfacePoint);
    algebra::GradientDescent<2> gradient_descent(std::move(function));

    gradient_descent.setStartingPoint(guess);
    auto result = *gradient_descent.calculate();
    auto pos = surface->value(result);

    if ((surfacePoint - pos).length() > kProjectionTolerance) {
      std::println("found too far point (length = {})",
                   (surfacePoint - pos).length());
      continue;
    }

    /// Increase learning rate to escape local minima
    if (i > kMaxIntersectionCurvePoint / 10) {
      gradient_descent.setLearningRate(gradient_descent.getLearningRate() *
                                       2.f);
    }

    return result;
  }
  return std::nullopt;
}

algebra::Vec3f
IntersectionFinder::getTangent(const IntersectionPoint &firstPoint) const {
  auto [du0, dv0] = surface0_->derivatives(firstPoint.surface0);
  auto [du1, dv1] = surface1_->derivatives(firstPoint.surface1);

  Vec3f n0 = du0.cross(dv0);
  Vec3f n1 = du1.cross(dv1);

  Vec3f tangent = n0.cross(n1);

  return tangent.normalize();
}

std::optional<Intersection>
IntersectionFinder::findNextPoints(const IntersectionPoint &firstPoint,
                                   bool reversed) const {
  std::vector<IntersectionPoint> points = {firstPoint};

  for (std::size_t i = 1; i < kMaxIntersectionCurvePoint; ++i) {
    auto next_point = nextIntersectionPoint(points.back(), reversed);

    if (i > 2 && intersectionLooped(Intersection{.points = points})) {
      points.back() = points.front();
      std::println("Intersection is looped!");
      return Intersection{.points = points, .looped = true};
    }

    if (next_point) {
      points.push_back(*next_point);
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

  const auto next_intersection = newton.calculate();

  if (next_intersection) {
    auto minimum = *next_intersection;
    auto surface_0_minimum = algebra::Vec2f{minimum[0], minimum[1]};
    auto surface_1_minimum = algebra::Vec2f{minimum[2], minimum[3]};

    auto surface_0_val = surface0_->value(surface_0_minimum);
    auto surface_1_val = surface1_->value(surface_1_minimum);

    return IntersectionPoint{.surface0 = surface_0_minimum,
                             .surface1 = surface_1_minimum,
                             .point = (surface_0_val + surface_1_val) / 2.f};
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

  if (!surface0_ || !surface1_) {
    return;
  }

  const auto bounds0 = surface0_->bounds();
  const auto bounds1 = surface1_->bounds();

  for (int i : {0, static_cast<int>(points.size()) - 1}) {
    snap(points[i].surface0[0], bounds0[0], bounds0[1], 0);
    snap(points[i].surface0[1], bounds0[0], bounds0[1], 1);
    snap(points[i].surface1[0], bounds1[0], bounds1[1], 0);
    snap(points[i].surface1[1], bounds1[0], bounds1[1], 1);
  }
}

bool IntersectionFinder::intersectionLooped(
    const Intersection &intersection) const {
  return false;
  const auto &first_point = intersection.points.front();
  const auto &last_point = intersection.points.back();
  auto surf_0_wrapped = surface0_->wrapped(0) || surface0_->wrapped(1);
  auto surf_1_wrapped = surface1_->wrapped(0) || surface1_->wrapped(1);

  const auto dist = 0.005f;
  return ((first_point.surface0 - last_point.surface0).length() < dist &&
          surf_0_wrapped) &&
         ((first_point.surface1 - last_point.surface1).length() < dist &&
          surf_1_wrapped);
}

algebra::Vec2f IntersectionFinder::findInitialGuessWithGuidance(
    const algebra::IDifferentialParametricForm<2, 3> *surface,
    const algebra::Vec3f &targetPoint, uint32_t gridResolution) const {
  auto bounds = surface->bounds();
  float min_dist_sq = std::numeric_limits<float>::max();
  algebra::Vec2f best_uv;

  for (int i = 0; i < gridResolution; ++i) {
    for (int j = 0; j < gridResolution; ++j) {
      float u = bounds[0][0] + (bounds[0][1] - bounds[0][0]) *
                                   (static_cast<float>(i) /
                                    static_cast<float>(gridResolution - 1));
      float v = bounds[1][0] + (bounds[1][1] - bounds[1][0]) *
                                   (static_cast<float>(j) /
                                    static_cast<float>(gridResolution - 1));

      auto p = surface->value({u, v});
      float dist_sq = std::pow((p - targetPoint).length(), 2.f);

      if (dist_sq < min_dist_sq) {
        min_dist_sq = dist_sq;
        best_uv = algebra::Vec2f(u, v);
      }
    }
  }
  return best_uv;
}