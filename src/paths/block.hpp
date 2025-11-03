#pragma once

struct Block {
  struct Dimensions {
    float x_;
    float y_;
    float z_;
  };

  Dimensions dimensions_;

  static Block defaultBlock() {
    return Block{.dimensions_{.x_ = 15.f, .y_ = 5.f, .z_ = 15.f}};
  }
};