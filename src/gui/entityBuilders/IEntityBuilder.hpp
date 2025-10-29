#pragma once

#include "IEntity.hpp"
#include "entityFactory.hpp"
#include "imgui.h"
#include <memory>

class IEntityBuilder {
public:
  explicit IEntityBuilder(GUI *gui, EntityFactory *factory)
      : gui_(gui), factory_(factory) {}
  virtual ~IEntityBuilder() = default;

  virtual void drawGui() {
    ImGui::SameLine();
    if (ImGui::Button("Create")) {
      create();
    }
  };
  virtual std::optional<IEntity *> create() const = 0;

protected:
  GUI *gui_;
  EntityFactory *factory_;
};