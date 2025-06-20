#include "intersectionFinder.hpp"
#include "gradientDescent.hpp"
#include "vec.hpp"

void IntersectionFinder::setSurfaces(
    std::shared_ptr<algebra::IDifferentialParametricForm> surface0,
    std::shared_ptr<algebra::IDifferentialParametricForm> surface1) {
  surface0_ = surface0;
  surface1_ = surface1;
}

void IntersectionFinder::setGuidancePoint(const algebra::Vec3f &guidancePoint) {
  guidancePoint_ = guidancePoint;
}

IntersectionPoint IntersectionFinder::findFirstPoint() const {
  algebra::GradientDescent gradientDescent(surface0_, surface1_);
  auto minimum = gradientDescent.calculate();

  auto surface0Minimum = algebra::Vec2f{minimum[0], minimum[1]};
  auto surface1Minimum = algebra::Vec2f{minimum[2], minimum[3]};

  auto surface0Val = surface0_.lock()->value(surface0Minimum);
  auto surface1Val = surface0_.lock()->value(surface1Minimum);

  // HERE MAYBE CHECK IF diff less than step

  return IntersectionPoint{.surface0 = surface0Minimum,
                           .surface1 = surface1Minimum,
                           .point = (surface0Val + surface1Val) / 2.f};
}