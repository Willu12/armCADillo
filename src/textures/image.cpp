#include "image.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

Image::Image(const std::string &path) : _path(path) {

  int width, height, bpp;
  auto buff = reinterpret_cast<uint32_t *>(
      stbi_load(path.c_str(), &width, &height, &bpp, 4));

  for (int i = 0; i < width * height; ++i) {
    swap_endianess(&buff[i]);
  }
  _width = width;
  _height = height;
  _bpp = bpp;

  if (!buff)
    throw std::runtime_error("could load texture from " + path + '\n');

  _data.insert(_data.end(), &buff[0], &buff[_width * _height]);
  stbi_image_free(buff);
}