#pragma once

#include "IEntity.hpp"
#include "entityBuilders/IEntityBuilder.hpp"
#include <memory>

class InterpolatingSplineBuilder : public IEntityBuilder {
public:
  InterpolatingSplineBuilder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override { IEntityBuilder::drawGui(); };
  std::optional<IEntity *> create() const override;
};