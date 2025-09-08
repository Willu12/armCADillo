#pragma once

#include "IEntity.hpp"
#include "entityBuilders/IEntityBuilder.hpp"
#include "entityFactory.hpp"
#include <memory>

class GregoryPatchBuilder : public IEntityBuilder {
public:
  GregoryPatchBuilder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override { IEntityBuilder::drawGui(); };

  std::optional<std::shared_ptr<IEntity>> create() const override;
};