#pragma once

#include "IEntity.hpp"
#include "IEntityFactory.hpp"
#include "torusEntity.hpp"
#include <memory>

class TorusFactory : public IEntityFactory {
public:
  std::shared_ptr<IEntity> create(const algebra::Vec3f &position) override {
    auto *raw = new TorusEntity(1.0, 0.3, position);
    return std::shared_ptr<IEntity>(raw);
  }
};