#pragma once

#include "vec.hpp"
#include <cstdint>
struct Color {
  uint8_t Red;
  uint8_t Green;
  uint8_t Blue;
  uint8_t Alpha;

  inline algebra::Vec4f convertToVector() {
    return algebra::Vec4f{
        static_cast<float>(Red) / 255.f, static_cast<float>(Green) / 255.f,
        static_cast<float>(Blue) / 255.f, static_cast<float>(Alpha) / 255.f};
  }

  inline bool operator==(const Color &other) const {
    return Red == other.Red && Green == other.Green && Blue == other.Blue &&
           Alpha == other.Alpha;
  }

  inline bool operator!=(const Color &other) const { return !(*this == other); }
};