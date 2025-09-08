#pragma once

#include "entityBuilders/IEntityBuilder.hpp"
#include "imgui.h"

class BezierSurfaceBuilder : public IEntityBuilder {
public:
  BezierSurfaceBuilder(GUI *gui, EntityFactory *factory)
      : IEntityBuilder(gui, factory) {}
  void drawGui() override {
    ImGui::Begin("Bezier Surface Options");
    ImGui::RadioButton("Flat", &cyllinder_, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Cylinder", &cyllinder_, 1);
    ImGui::Separator();
    ImGui::InputInt("U Patches", &uPatches_);
    ImGui::InputInt("V Patches", &vPatches_);

    if (cyllinder_ == 0) {
      ImGui::InputFloat("uLen", &x_);
      ImGui::InputFloat("VLen", &y_);
    } else {
      ImGui::InputFloat("Radius", &x_);
      ImGui::InputFloat("Height", &y_);
    }

    ImGui::Separator();
    ImGui::End();

    IEntityBuilder::drawGui();
  };

protected:
  int uPatches_ = 2;
  int vPatches_ = 2;

  float x_ = 1.0f;
  float y_ = 2.0f;
  int cyllinder_ = false;
};