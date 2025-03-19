#pragma once
#include "imgui.h"
#include "torusModel.hpp"

class TorusSettings {
public:
  TorusModel *_torusModel;
  bool _change;

  TorusSettings(TorusModel *torusModel) : _torusModel(torusModel){};

  void ShowSettingsWindow() {
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", nullptr, window_flags)) {

      _change |=
          ImGui::SliderFloat("R", &_torusModel->getInnerRadius(), 0.1f, 10.f);
      _change |=
          ImGui::SliderFloat("r", &_torusModel->getTubeRadius(), 0.1f, 10.f);
      _change |= ImGui::SliderInt("Horizontal Density",
                                  &_torusModel->getMeshDensity().s, 3, 100);
      _change |= ImGui::SliderInt("Vertical Density",
                                  &_torusModel->getMeshDensity().t, 3, 100);
      ImGui::End();
    }
  }

private:
};