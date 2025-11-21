#pragma once

#include "imgui.h"
class IntersectionConfig {
public:
  bool useCursor_ = false;
  float numericalStep_ = 0.001f;
  float intersectionStep_ = 0.01f;
  bool useOffsetSurface_ = true;
  float offsetValue_ = 0.4f;

  void display() {
    if (ImGui::Begin("Intersection Settings")) {
      ImGui::Checkbox("Use cursor", &useCursor_);
      ImGui::SliderFloat("Numerical step", &numericalStep_, kNumericalStepMin,
                         kNumericalStepMax, "%.4f");
      ImGui::SliderFloat("Intersection step", &intersectionStep_,
                         kIntersectionStepMin, kIntersectionStepMax, "%.4f");

      ImGui::Checkbox("Use offset surface", &useOffsetSurface_);

      ImGui::BeginDisabled(!useOffsetSurface_);
      ImGui::InputFloat("Offset value", &offsetValue_);
      ImGui::EndDisabled();
    }
    ImGui::End();
  }

private:
  static float constexpr kNumericalStepMin = 0.0001f;
  static float constexpr kNumericalStepMax = 0.01f;
  static float constexpr kIntersectionStepMin = 0.001f;
  static float constexpr kIntersectionStepMax = 1.0f;
};