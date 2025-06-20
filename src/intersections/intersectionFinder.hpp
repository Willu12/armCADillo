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
  void setSurfaces(std::shared_ptr<const IDifferentialParametricForm> surface0,
                   std::shared_ptr<const IDifferentialParametricForm> surface1);
  void setGuidancePoint(const algebra::Vec3f &guidancePoint);

  IntersectionPoint findFirstPoint() const;

private:
  std::weak_ptr<const IDifferentialParametricForm> surface0_;
  std::weak_ptr<const IDifferentialParametricForm> surface1_;
  std::optional<algebra::Vec3f> guidancePoint_;
};