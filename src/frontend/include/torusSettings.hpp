#pragma once
#include "imgui.h"
#include "modelController.hpp"
#include "torusModel.hpp"

class TorusSettings {
public:
  enum ControllerKind { Camera = 0, Model = 1 };

  TorusModel *_torusModel;
  bool _change;
  ControllerKind _controllerKind = Camera;
  ModelController *_modelController;

  TorusSettings(TorusModel *torusModel, ModelController *modelController)
      : _torusModel(torusModel), _modelController(modelController) {}

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

      const char *controllerOptions[] = {"Camera", "Model"};
      int selectedIndex = static_cast<int>(_controllerKind);

      if (ImGui::Combo("Controller", &selectedIndex, controllerOptions,
                       IM_ARRAYSIZE(controllerOptions))) {
        _controllerKind = static_cast<ControllerKind>(selectedIndex);
        //_change = true;
      }
      if (_controllerKind == TorusSettings::ControllerKind::Model) {
        const char *AxisOptions[] = {"X axis", "Y axis", "Z axis"};
        selectedIndex = static_cast<int>(_modelController->_transformationAxis);
        if (ImGui::Combo("TransformationAxis", &selectedIndex, AxisOptions,
                         IM_ARRAYSIZE(AxisOptions))) {
          _modelController->_transformationAxis =
              static_cast<Axis>(selectedIndex);
        }
      }

      ImGui::End();
    }
  }

private:
};