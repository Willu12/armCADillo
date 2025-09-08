#pragma once

#include "IEntity.hpp"
#include "bezierSurfaceBuilder.hpp"
#include <memory>

class BezierSurfaceC2Builder : public BezierSurfaceBuilder {
public:
  BezierSurfaceC2Builder(GUI *gui, EntityFactory *factory)
      : BezierSurfaceBuilder(gui, factory) {}
  void drawGui() override { BezierSurfaceBuilder::drawGui(); };

  std::optional<std::shared_ptr<IEntity>> create() const override;
};