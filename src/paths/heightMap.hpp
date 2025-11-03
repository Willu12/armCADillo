#pragma once

#include "block.hpp"
#include <cstdint>
#include <vector>

struct Divisions {
  uint32_t x_ = 1500;
  uint32_t z_ = 1500;
};

class HeightMap {
public:
  HeightMap(Divisions divisions, float baseHeight, const Block *block)
      : divisions_(divisions), baseHeight_(baseHeight), block_(block) {}

  const Divisions &divisions() const { return divisions_; }
  float baseHeight() const { return baseHeight_; }
  float &at(uint32_t index) { return data_[index]; }
  float at(uint32_t index) const { return data_[index]; }
  const Block &block() const { return *block_; }

private:
  Divisions divisions_;
  float baseHeight_;
  const Block *block_;

  std::vector<float> data_ =
      std::vector<float>(divisions_.x_ * divisions_.z_, baseHeight_);
};