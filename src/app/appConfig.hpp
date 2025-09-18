#pragma once

#include "imgui.h"
#include <cstdint>

struct AppConfig {
  static constexpr uint32_t kHeight = 1080;
  static constexpr uint32_t kWidth = 1920;
  static constexpr const char *kGlslVersion = "#version 430";
  static constexpr const char *kName = "Armcadillo";
  static constexpr ImVec4 kClearColor = ImVec4(0.1, 0.1f, 0.1f, 1.00f);
};