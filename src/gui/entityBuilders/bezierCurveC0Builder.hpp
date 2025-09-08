#pragma once

#include "IEntity.hpp"
#include "entityBuilders/IEntityBuilder.hpp"
#include <memory>

class BezierCurveC0Builder : public IEntityBuilder {
public:
  BezierCurveC0Builder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override { IEntityBuilder::drawGui(); };
  std::optional<std::shared_ptr<IEntity>> create() const override;
};