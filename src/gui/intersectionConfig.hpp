#pragma once

#include "imgui.h"
class IntersectionConfig {
public:
  bool useCursor_ = false;
  float numericalStep_ = 0.001f;
  float intersectionStep_ = 0.01f;

  void display() {
    if (ImGui::Begin("Intersection Settings")) {
      ImGui::Checkbox("Use Cursor", &useCursor_);
      ImGui::SliderFloat("Numerical Step", &numericalStep_, kNumericalStepMin,
                         kNumericalStepMax, "%.4f");
      ImGui::SliderFloat("Intersection Step", &intersectionStep_,
                         kIntersectionStepMin, kIntersectionStepMax, "%.4f");
    }
    ImGui::End();
  }

private:
  static float constexpr kNumericalStepMin = 0.0001f;
  static float constexpr kNumericalStepMax = 0.01f;
  static float constexpr kIntersectionStepMin = 0.001f;
  static float constexpr kIntersectionStepMax = 1.0f;
};