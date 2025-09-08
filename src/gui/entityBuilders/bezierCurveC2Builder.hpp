#pragma once

#include "IEntity.hpp"
#include "entityBuilders/IEntityBuilder.hpp"
#include <memory>

class BSplineCurveBuilder : public IEntityBuilder {
public:
  BSplineCurveBuilder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override { IEntityBuilder::drawGui(); };
  std::optional<std::shared_ptr<IEntity>> create() const override;
};