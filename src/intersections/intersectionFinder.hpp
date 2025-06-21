#pragma once

#include "IDifferentialParametricForm.hpp"
#include "vec.hpp"
#include <memory>

struct IntersectionPoint {
  algebra::Vec2f surface0;
  algebra::Vec2f surface1;
  algebra::Vec3f point;
};

class IntersectionFinder {
public:
  void setSurfaces(
      std::shared_ptr<algebra::IDifferentialParametricForm<2, 3>> surface0,
      std::shared_ptr<algebra::IDifferentialParametricForm<2, 3>> surface1);
  void setGuidancePoint(const algebra::Vec3f &guidancePoint);

  std::optional<IntersectionPoint> findFirstPoint() const;

private:
  std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface0_;
  std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface1_;
  std::optional<algebra::Vec3f> guidancePoint_;

  std::optional<IntersectionPoint>
  findCommonSurfacePoint(const algebra::Vec2f &start0,
                         const algebra::Vec2f &start1) const;
  algebra::Vec2f findPointProjection(
      std::weak_ptr<algebra::IDifferentialParametricForm<2, 3>> surface,
      algebra::Vec3f surfacePoint) const;
  static constexpr std::size_t kStochasticTries = 10;
};