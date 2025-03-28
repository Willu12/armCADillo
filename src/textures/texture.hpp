#pragma once
#include "textureResource.hpp"
enum class TextureFiltering { Linear, Nearest };

enum class TextureWrapping {
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder,
};
class Texture {
public:
  explicit Texture(const TextureResource &textureResource)
      : _textureResource(textureResource) {}

  void bind(uint32_t unit) const {
    _textureResource.bind(unit);
    if (_filtering == TextureFiltering::Linear) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    if (_wrapping == TextureWrapping::Repeat) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else if (_wrapping == TextureWrapping::ClampToEdge) {

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else if (_wrapping == TextureWrapping::ClampToBorder) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    }
  }

private:
  const TextureResource &_textureResource;
  TextureFiltering _filtering = TextureFiltering::Linear;
  TextureWrapping _wrapping = TextureWrapping::Repeat;
};