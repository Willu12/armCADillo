#pragma once
#include "IEntity.hpp"
#include "memory"
#include "vec.hpp"

class IEntityFactory {
public:
  virtual ~IEntityFactory() = default;
  virtual std::shared_ptr<IEntity> create(const algebra::Vec3f &position) = 0;
};