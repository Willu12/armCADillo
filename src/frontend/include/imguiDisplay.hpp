#pragma once
#include "imgui.h"
#include "torusModel.hpp"

class TorusSettings {
public:
  TorusModel *torusModel;
  bool change;

  void ShowSettingsWindow() {
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", nullptr, window_flags)) {

      change |=
          ImGui::SliderFloat("R", &torusModel->getInnerRadius(), 0.1f, 10.f);
      change |=
          ImGui::SliderFloat("r", &torusModel->getTubeRadius(), 0.1f, 10.f);
      change |= ImGui::SliderInt("Horizontal Density",
                                 &torusModel->getMeshDensity().s, 3, 100);
      change |= ImGui::SliderInt("Vertical Density",
                                 &torusModel->getMeshDensity().t, 3, 100);
    }
  }

private:
};