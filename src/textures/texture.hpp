#pragma once
#include "intersectionFinder.hpp"
#include "textureResource.hpp"
#include <cstdint>
#include <memory>
#include <ranges>
enum class TextureFiltering { Linear, Nearest };

enum class TextureWrapping {
  Repeat,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder,
};
class Texture {
public:
  explicit Texture(TextureResource textureResource)
      : _textureResource(textureResource) {}

  static std::unique_ptr<Texture> createTexture(const std::string &imagePath) {
    Image image(imagePath);
    TextureResource textureResource(image);
    Texture *texture = new Texture(textureResource);
    return std::unique_ptr<Texture>(texture);
  }

  static std::unique_ptr<Texture> createTexture(uint32_t width,
                                                uint32_t height) {
    TextureResource textureResource(width, height);
    Texture *texture = new Texture(textureResource);
    return std::unique_ptr<Texture>(texture);
  }

  void fill(const std::vector<uint8_t> &canvas) {
    _textureResource.fill(canvas);
  }

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

  uint32_t getTextureId() const { return _textureResource.getTextureId(); }

private:
  TextureResource _textureResource;
  TextureFiltering _filtering = TextureFiltering::Linear;
  TextureWrapping _wrapping = TextureWrapping::Repeat;
};
