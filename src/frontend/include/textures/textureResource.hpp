#pragma once
#include "GLFW/glfw3.h"
#include <memory>

class TextureResource {
public:
  TextureResource() {
    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);

    setWrappingParameters();
  }

private:
  GLuint _id;

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