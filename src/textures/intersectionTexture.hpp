#pragma once
#include "texture.hpp"
class IntersectionTexture {
public:
  static std::pair<std::unique_ptr<IntersectionTexture>,
                   std::unique_ptr<IntersectionTexture>>
  createIntersectionTextures(const Intersection &intersection) {
    std::vector<algebra::Vec2f> points1(intersection.points.size());
    std::vector<algebra::Vec2f> points2(intersection.points.size());

    for (const auto &[i, p] :
         intersection.points | std::ranges::views::enumerate) {
      points1[i] = p.surface0;
      points2[i] = p.surface1;
    }

    auto texture1 = std::make_unique<IntersectionTexture>(points1);
    auto texture2 = std::make_unique<IntersectionTexture>(points2);
    return {std::move(texture1), std::move(texture2)};
  }

  explicit IntersectionTexture(
      const std::vector<algebra::Vec2f> &surfacePoints) {
    texture_ = Texture::createTexture(kWidth, kHeight);
    fillCanvas(surfacePoints);
    texture_->fill(canvas_);
  }

  uint32_t getTextureId() const { return texture_->getTextureId(); }

private:
  static int constexpr kWidth = 300;
  static int constexpr kHeight = 300;

  std::unique_ptr<Texture> texture_;
  std::vector<uint8_t> canvas_;

  void fillCanvas(const std::vector<algebra::Vec2f> &surfacePoints) {
    // Resize and clear: black and fully opaque (optional alpha)
    canvas_.assign(kWidth * kHeight * 4, 0); // All black, RGBA = (0,0,0,0)

    auto toPixel = [&](const algebra::Vec2f &p) -> std::pair<int, int> {
      int x = std::clamp(static_cast<int>(p[0] * kWidth), 0, kWidth - 1);
      int y = std::clamp(static_cast<int>(p[1] * kHeight), 0, kHeight - 1);

      return {x, y};
    };

    for (size_t i = 1; i < surfacePoints.size(); ++i) {
      auto [x0, y0] = toPixel(surfacePoints[i - 1]);
      auto [x1, y1] = toPixel(surfacePoints[i]);

      int dx = std::abs(x1 - x0);
      int dy = std::abs(y1 - y0);
      int steps = std::max(dx, dy);
      if (steps == 0)
        steps = 1;

      float stepX = (x1 - x0) / static_cast<float>(steps);
      float stepY = (y1 - y0) / static_cast<float>(steps);

      float x = static_cast<float>(x0);
      float y = static_cast<float>(y0);

      for (int s = 0; s <= steps; ++s) {
        int ix = std::clamp(static_cast<int>(x), 0, kWidth - 1);
        int iy = std::clamp(static_cast<int>(y), 0, kHeight - 1);
        int index = 4 * (iy * kWidth + ix);

        // Set to white (255,255,255,255)
        canvas_[index + 0] = 255; // Red
        canvas_[index + 1] = 255; // Green
        canvas_[index + 2] = 255; // Blue
        canvas_[index + 3] = 255; // Alpha (opaque)

        x += stepX;
        y += stepY;
      }
    }
  }
};