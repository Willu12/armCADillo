#pragma once
#include <glad/gl.h>

#include "GLFW/glfw3.h"
#include "image.hpp"
#include <memory>

class TextureResource {
public:
  TextureResource(const uint32_t width, const uint32_t height)
      : _width(width), _height(height) {
    glGenTextures(1, &_id);
    setWrappingParameters();
  };
  TextureResource(const Image &image)
      : _width(image.getWidth()), _height(image.getHeight()), bpp(4) {

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    setWrappingParameters();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8, image.raw());
  }

  void bind(uint32_t unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, _id);
    setWrappingParameters();
  }

  void fill(const std::vector<uint8_t> &canvas) {
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, canvas.data());
    setWrappingParameters();
  }

  GLuint getTextureId() const { return _id; }

private:
  GLuint _id{};
  int _width, _height, bpp;

  void setWrappingParameters() const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
};