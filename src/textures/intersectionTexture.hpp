#pragma once
#include "canvas.hpp"
#include "texture.hpp"
#include "vec.hpp"
#include <cstdint>
#include <vector>

class IntersectionTexture {
public:
  static std::pair<std::shared_ptr<IntersectionTexture>,
                   std::shared_ptr<IntersectionTexture>>
  createIntersectionTextures(
      const Intersection &intersection,
      std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>
          bounds);

  void bind() const;
  explicit IntersectionTexture(const std::vector<algebra::Vec2f> &surfacePoints,
                               const std::array<algebra::Vec2f, 2> &bounds);
  uint32_t getTextureId() const;
  void floodFill(uint32_t x, uint32_t y, bool transparent);
  bool wrapU() const { return wrapU_; }
  bool wrapV() const { return wrapV_; }
  void setWrapping(bool wrapU, bool wrapV) {
    wrapU_ = wrapU;
    wrapV_ = wrapV;
  }

private:
  static int constexpr kWidth = 300;
  static int constexpr kHeight = 300;
  bool wrapU_ = false;
  bool wrapV_ = false;
  std::unique_ptr<Texture> texture_;
  std::array<algebra::Vec2f, 2> bounds_;
  Canvas canvas_;

  void drawLine(const std::vector<algebra::Vec2f> &surfacePoints);
  void fillCanvas(const std::vector<algebra::Vec2f> &surfacePoints);
};
