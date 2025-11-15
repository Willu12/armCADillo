#pragma once
#include "canvas.hpp"
#include "texture.hpp"
#include "vec.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class IntersectionTexture {
public:
  struct Dimensions {
    int height;
    int width;
  };

  enum class CellType : uint8_t { Keep, Intersection, Trim };

  static std::pair<std::unique_ptr<IntersectionTexture>,
                   std::unique_ptr<IntersectionTexture>>
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

  algebra::Vec2f uv(uint32_t x, uint32_t y) const;

  Dimensions getSize() const {
    return Dimensions{.height = kHeight, .width = kWidth};
  }

  void setCellType(uint32_t x, uint32_t y, CellType cellType);
  CellType getCellType(uint32_t x, uint32_t y) const;
  bool isTrimmed(uint32_t x, uint32_t y) const;

  void update();

private:
  static int constexpr kWidth = 800;
  static int constexpr kHeight = 800;
  bool wrapU_ = false;
  bool wrapV_ = false;
  std::unique_ptr<Texture> texture_;
  std::array<algebra::Vec2f, 2> bounds_;
  Canvas canvas_;

  void drawLine(const std::vector<algebra::Vec2f> &surfacePoints);
  void fillCanvas(const std::vector<algebra::Vec2f> &surfacePoints);
};
