#pragma once

#include "color.hpp"
#include <cstdint>
#include <cstring>
#include <vector>
class Canvas {
public:
  Canvas(uint32_t width, uint32_t height) : width_(width), height_(height) {
    data_ = std::vector<uint8_t>(4 * width_ * height_);
  }

  void fillWithColor(const Color &color) {
    for (int i = 0; i < width_ * height_; ++i)
      fillAtIndex(i, color);
  }

  void fillAtIndex(std::size_t index, const Color &color) {
    data_[4 * index] = color.Red;
    data_[4 * index + 1] = color.Green;
    data_[4 * index + 2] = color.Blue;
    data_[4 * index + 3] = color.Alpha;
  }

  Color colorAtIndex(std::size_t index) const {
    return Color{.Red = data_[4 * index],
                 .Green = data_[4 * index + 1],
                 .Blue = data_[4 * index + 2],
                 .Alpha = data_[4 * index + 3]};
  }
  void flipY() {
    std::vector<uint8_t> new_data(data_.size());
    const int rowSize = width_ * 4;

    for (uint32_t y = 0; y < height_; ++y) {
      const uint8_t *srcRow = &data_[(height_ - 1 - y) * rowSize];
      uint8_t *dstRow = &new_data[y * rowSize];
      memcpy(dstRow, srcRow, rowSize);
    }

    data_ = std::move(new_data);
  }
  std::vector<uint8_t> &getData() { return data_; }

private:
  std::vector<uint8_t> data_;
  uint32_t width_;
  uint32_t height_;
};