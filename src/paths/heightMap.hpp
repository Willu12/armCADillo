#pragma once

#include <cstdint>
#include <vector>

struct Divisions {
  uint32_t x_ = 1500;
  uint32_t y_ = 1500;
};

class HeightMap {
public:
  HeightMap(Divisions divisions, float baseHeight)
      : divisions_(divisions), baseHeight_(baseHeight) {}

private:
  Divisions divisions_;
  float baseHeight_;

  std::vector<float> data_ =
      std::vector<float>(divisions_.x_ * divisions_.y_, baseHeight_);
};