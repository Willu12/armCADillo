#pragma once

#include "IDifferentialParametricForm.hpp"
#include "intersectionConfig.hpp"
#include "vec.hpp"

#include <memory>
#include <optional>
#include <vector>

struct IntersectionPoint {
  algebra::Vec2f surface0;
  algebra::Vec2f surface1;
  algebra::Vec3f point;
};

struct Intersection {
  std::vector<IntersectionPoint> points;
  bool looped = false;
  algebra::Vec3f firstPoint;
};

class IntersectionFinder {
public:
  void setSurfaces(algebra::IDifferentialParametricForm<2, 3> *surface0,
                   algebra::IDifferentialParametricForm<2, 3> *surface1);
  void setGuidancePoint(const algebra::Vec3f &guidancePoint);
  std::optional<Intersection> find(bool same) const;

  IntersectionConfig &getIntersectionConfig() { return config_; }

private:
  algebra::IDifferentialParametricForm<2, 3> *surface0_;
  algebra::IDifferentialParametricForm<2, 3> *surface1_;
  std::optional<algebra::Vec3f> guidancePoint_;
  static constexpr std::size_t kStochasticTries = 300;
  static constexpr std::size_t kMaxIntersectionCurvePoint = 2000;

  mutable IntersectionConfig config_;

  std::optional<Intersection>
  findNextPoints(const IntersectionPoint &firstPoint, bool reversed) const;
  std::optional<IntersectionPoint> findFirstPoint(bool same) const;
  std::optional<IntersectionPoint> findFirstPointStochastic() const;
  std::optional<IntersectionPoint> findFirstPointSameStochastic() const;
  std::optional<IntersectionPoint> findFirstPointWithGuidance() const;
  std::optional<IntersectionPoint> findFirstPointSameWithGuidance() const;
  std::optional<IntersectionPoint>
  newtowRefinment(const IntersectionPoint &point) const;
  std::optional<IntersectionPoint>
  findCommonSurfacePoint(const algebra::Vec2f &start0,
                         const algebra::Vec2f &start1) const;
  std::optional<algebra::Vec2f>
  findPointProjection(algebra::IDifferentialParametricForm<2, 3> *surface,
                      algebra::Vec3f surfacePoint) const;

  algebra::Vec2f findInitialGuessWithGuidance(
      algebra::IDifferentialParametricForm<2, 3> *surface,
      const algebra::Vec3f &targetPoint, uint32_t gridResolution = 10) const;

  algebra::Vec3f getTangent(const IntersectionPoint &firstPoint) const;
  std::optional<IntersectionPoint>
  nextIntersectionPoint(const IntersectionPoint &lastPoint,
                        bool reversed) const;

  std::optional<Intersection>
  connectFoundPoints(const std::optional<Intersection> &nextPoints,
                     const std::optional<Intersection> &previousPoints) const;

  void fixIntersectionPointsEdges(std::vector<IntersectionPoint> &points) const;
  bool intersectionLooped(const Intersection &intersection) const;
};