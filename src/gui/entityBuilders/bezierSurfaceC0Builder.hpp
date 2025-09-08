#pragma once

#include "IEntity.hpp"
#include "entityBuilders/bezierSurfaceBuilder.hpp"
#include <memory>

class BezierSurfaceC0Builder : public BezierSurfaceBuilder {
public:
  BezierSurfaceC0Builder(GUI *gui, EntityFactory *factory)
      : BezierSurfaceBuilder(gui, factory) {}
  void drawGui() override { BezierSurfaceBuilder::drawGui(); };

  std::optional<std::shared_ptr<IEntity>> create() const override;
};