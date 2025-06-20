#include "intersectionFinder.hpp"

void IntersectionFinder::setSurfaces(
    std::shared_ptr<const IDifferentialParametricForm> surface0,
    std::shared_ptr<const IDifferentialParametricForm> surface1) {
  surface0_ = surface0;
  surface1_ = surface1;
}

void IntersectionFinder::setGuidancePoint(const algebra::Vec3f &guidancePoint) {
  guidancePoint_ = guidancePoint;
}

IntersectionPoint IntersectionFinder::findFirstPoint() const {}