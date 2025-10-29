#pragma once

#include "IEntity.hpp"
#include "entityBuilders/IEntityBuilder.hpp"

class PointBuilder : public IEntityBuilder {
public:
  PointBuilder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override { IEntityBuilder::drawGui(); };

  std::optional<IEntity *> create() const override;
};