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

private:
  static int constexpr kWidth = 512;
  static int constexpr kHeight = 512;

  std::unique_ptr<Texture> texture_;
  std::array<algebra::Vec2f, 2> bounds_;
  Canvas canvas_;

  void drawLine(const std::vector<algebra::Vec2f> &surfacePoints);
  std::vector<std::pair<uint32_t, uint32_t>> findInteriorSamplePoints();
  std::vector<std::vector<algebra::Vec2f>>
  divideSurfacePoints(const std::vector<algebra::Vec2f> &surfacePoints);
  void fillCanvas(const std::vector<algebra::Vec2f> &surfacePoints);
  void floodFillTransparent(int startX, int startY);
};
