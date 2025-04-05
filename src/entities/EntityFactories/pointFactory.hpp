#pragma once

#include "IEntity.hpp"
#include "IEntityFactory.hpp"
#include "pointEntity.hpp"
#include <memory>

class PointFactory : public IEntityFactory {
public:
  std::shared_ptr<IEntity> create(const algebra::Vec3f &position) override {
    PointEntity *raw = new PointEntity(position);
    return std::shared_ptr<IEntity>(raw);
  }
};