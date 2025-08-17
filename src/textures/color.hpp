#pragma once

#include "vec.hpp"
#include <cstdint>
struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

  algebra::Vec4f convertToVector() const {
    return algebra::Vec4f{
        static_cast<float>(r) / 255.f, static_cast<float>(g) / 255.f,
        static_cast<float>(b) / 255.f, static_cast<float>(a) / 255.f};
  }

  bool operator==(const Color &other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }

  bool operator!=(const Color &other) const { return !(*this == other); }

  static constexpr Color Red() { return Color(255, 0, 0, 255); }
  static constexpr Color Blue() { return Color(0, 0, 255, 255); }
  static constexpr Color Green() { return Color(0, 255, 0, 255); }
  static constexpr Color Orange() { return Color(204, 85, 0, 255); }
  static constexpr Color White() { return Color(255, 255, 255, 255); }
  static constexpr Color Black() { return Color(0, 0, 0, 255); }
  static constexpr Color Transparent() { return Color(0, 0, 0, 0); }
};