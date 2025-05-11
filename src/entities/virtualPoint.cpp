#include "virtualPoint.hpp"
#include "bSplineCurve.hpp"

void VirtualPoint::updatePosition(const algebra::Vec3f &position) {

  auto oldPosition = getPosition();
  IEntity::updatePosition(position);

  for (auto &subscriber : _subscribers) {
    auto *bSplineCurve = dynamic_cast<BSplineCurve *>(&subscriber.get());

    if (bSplineCurve) {
      bSplineCurve->updateBezier(*this, oldPosition);
      bSplineCurve->updateMesh();
    } else
      throw std::runtime_error("failed to dynamic_cast to BSplineCurve");
  }
  // notifySubscribers();
}