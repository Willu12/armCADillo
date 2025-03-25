#pragma once
#include "GLFW/glfw3.h"
#include "image.hpp"
#include <memory>

class TextureResource {
public:
  TextureResource(const Image &image)
      : _id(0), _width(image.getWidth()), _height(image.getHeight()), bpp(4) {

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    setWrappingParameters();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8, image.raw());
  }

private:
  GLuint _id;
  int _width, _height, bpp;

  void setWrappingParameters() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT); // set texture wrapping to GL_REPEAT (default
                                // wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
};