#pragma once
#include "glad/glad.h"
#include <cstdint>
#include <stdexcept>

class PickingTexture {
public:
  //  PickingTexture() {}
  ~PickingTexture() {
    if (_fbo != 0) {
      glDeleteFramebuffers(1, &_fbo);
    }

    if (_pickingTexture != 0) {
      glDeleteTextures(1, &_pickingTexture);
    }

    if (_depthTexture != 0) {
      glDeleteTextures(1, &_depthTexture);
    }
  }

  void init(unsigned int windowWidth, unsigned int windowHeight) {
    // create fbo
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    // create the texture object for the primitive information buferr
    glGenTextures(1, &_pickingTexture);
    glBindTexture(GL_TEXTURE_2D, _pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, windowWidth, windowHeight, 0,
                 GL_RGB_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           _pickingTexture, 0);

    // create the texture object for the depth buffer
    glGenTextures(1, &_depthTexture);
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth,
                 windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("Frame Buffer error, status " + status + '\n');

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void enableWriting() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo); }
  void disableWrtiing() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); };

  struct PixelInfo {
    uint32_t ObjectId = 0;
    uint32_t drawId = 0;
    uint32_t primId = 0;
  };

  PixelInfo ReadPixel(uint32_t x, uint32_t y) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    PixelInfo pixel;
    glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &pixel);

    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return pixel;
  }

private:
  uint32_t _fbo = 0;
  uint32_t _pickingTexture = 0;
  uint32_t _depthTexture = 0;
};