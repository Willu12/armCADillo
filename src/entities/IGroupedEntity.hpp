#pragma once

#include "IEntity.hpp"
#include <functional>
class PointEntity;

class IGroupedEntity : public IEntity {
public:
  virtual std::vector<std::reference_wrapper<const PointEntity>>
  getPointsReferences() const = 0;
  virtual std::vector<std::reference_wrapper<PointEntity>> &
  getPointsReferences() = 0;

private:
};