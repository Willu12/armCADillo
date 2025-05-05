#include "virtualPoint.hpp"
#include "bezierCurveC2.hpp"

// void VirtualPoint::notifySubscribers() {}
void VirtualPoint::updatePosition(const algebra::Vec3f &position) {

  auto oldPosition = getPosition();
  IEntity::updatePosition(position);

  notifySubscribers();
  for (auto &subscriber : _subscribers) {
    auto *bezierCurveC2 = dynamic_cast<BezierCurveC2 *>(&subscriber.get());

    if (bezierCurveC2) {
      bezierCurveC2->updateBezier(*this, oldPosition);
    } else
      throw std::runtime_error("failed to dynamic_cast to BezierCurveC2");
  }
}