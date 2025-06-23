#pragma once

#include "IDifferentialParametricForm.hpp"
#include "intersectionConfig.hpp"
#include "vec.hpp"

#include <memory>
#include <optional>

struct IntersectionPoint {
  algebra::Vec2f surface0;
  algebra::Vec2f surface1;
  algebra::Vec3f point;
};

struct Intersection {
  std::vector<IntersectionPoint> points;
};

class IntersectionFinder {
public:
  void setSurfaces(
      std::shared_ptr<algebra::IDifferentialParametricForm<2, 3>> surface0,
      std::shared_ptr<algebra::IDifferentialParametricForm<2, 3>> surface1);
  void setGuidancePoint(const algebra::Vec3f &guidancePoint);
  std::optional<Intersection> find() const;

  IntersectionConfig &getIntersectionConfig() { return config_; }

private:
  std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface0_;
  std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface1_;
  std::optional<algebra::Vec3f> guidancePoint_;
  static constexpr std::size_t kStochasticTries = 20;
  static constexpr std::size_t kMaxIntersectionCurvePoint = 200;

  IntersectionConfig config_;

  std::optional<Intersection>
  findNextPoints(const IntersectionPoint &firstPoint, bool reversed) const;
  std::optional<IntersectionPoint> findFirstPoint() const;

  std::optional<IntersectionPoint>
  findCommonSurfacePoint(const algebra::Vec2f &start0,
                         const algebra::Vec2f &start1) const;
  algebra::Vec2f findPointProjection(
      std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface,
      algebra::Vec3f surfacePoint) const;

  algebra::Vec3f getTangent(const IntersectionPoint &firstPoint) const;

  std::optional<IntersectionPoint>
  nextIntersectionPoint(const IntersectionPoint &lastPoint,
                        bool reversed) const;

  std::optional<Intersection>
  connectFoundPoints(const std::optional<Intersection> &nextPoints,
                     const std::optional<Intersection> &previousPoints) const;
};