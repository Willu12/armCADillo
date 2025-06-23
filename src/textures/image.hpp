#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

class Image {
public:
  explicit Image(const std::string &path);
  uint32_t getHeight() const { return _height; }
  uint32_t getWidth() const { return _width; }
  const uint32_t *raw() const { return _data.data(); }

private:
  uint32_t _width, _height, _bpp;
  std::vector<uint32_t> _data;
  std::string _path;

  void swap_endianess(uint32_t *pixel) {
    *pixel = ((*pixel & 0xFF000000) >> 24) | ((*pixel & 0x00FF0000) >> 8) |
             ((*pixel & 0x0000FF00) << 8) | ((*pixel & 0x000000FF) << 24);
  }
};