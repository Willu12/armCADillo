#pragma once
#include "canvas.hpp"
#include "color.hpp"
#include "texture.hpp"
#include "vec.hpp"
#include <cstdint>
#include <memory>
#include <print>
#include <vector>

class IntersectionTexture {
public:
  struct Dimensions {
    int height;
    int width;
  };

  struct Coord {
    uint32_t x;
    uint32_t y;
  };

  struct Segment {
    Coord start;
    Coord end;
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
  static Coord uvToCoord(const algebra::Vec2f &uv);

  Dimensions getSize() const {
    return Dimensions{.height = kHeight, .width = kWidth};
  }

  void setCellType(uint32_t x, uint32_t y, CellType cellType);
  CellType getCellType(uint32_t x, uint32_t y) const;
  void setColor(uint32_t x, uint32_t y, Color color);
  bool isTrimmed(uint32_t x, uint32_t y) const;

  void update();

  std::vector<Segment> &getIntersectionCurve() { return intersectionCurve_; }
  void setIntersectionCurve(const std::vector<Segment> &intersectionCurve) {
    intersectionCurve_ = intersectionCurve;
  }

  void closeIntersectionCurve();

  void drawLine(const std::vector<algebra::Vec2f> &surfacePoints,
                Color color = Color::Green());

private:
  static int constexpr kWidth = 1500;
  static int constexpr kHeight = 1500;
  bool wrapU_ = false;
  bool wrapV_ = false;
  std::unique_ptr<Texture> texture_;
  std::array<algebra::Vec2f, 2> bounds_;
  Canvas canvas_;
  std::vector<Segment> intersectionCurve_;

  void fillCanvas(const std::vector<algebra::Vec2f> &surfacePoints);
};
