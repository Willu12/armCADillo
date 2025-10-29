#pragma once

#include "IEntity.hpp"
#include "entityBuilders/IEntityBuilder.hpp"
#include <memory>

class TorusBuilder : public IEntityBuilder {
public:
  TorusBuilder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override { IEntityBuilder::drawGui(); };
  std::optional<IEntity *> create() const override;

private:
  float radius_ = 1.0f;
  float tubeRadius_ = 0.25f;
};