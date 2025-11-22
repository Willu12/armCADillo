#include "intersectionTexture.hpp"
#include "color.hpp"
#include "vec.hpp"
#include <memory>
#include <print>
#include <queue>

std::pair<std::unique_ptr<IntersectionTexture>,
          std::unique_ptr<IntersectionTexture>>
IntersectionTexture::createIntersectionTextures(
    const Intersection &intersection,
    std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>
        bounds) {
  std::vector<algebra::Vec2f> points0(intersection.points.size());
  std::vector<algebra::Vec2f> points1(intersection.points.size());

  for (const auto &[i, p] :
       intersection.points | std::ranges::views::enumerate) {
    points0[i] = p.surface0;
    points1[i] = p.surface1;
  }

  /// Intersection Curve
  auto start_surface_0 = uvToCoord(intersection.points.front().surface0);
  auto start_surface_1 = uvToCoord(intersection.points.front().surface1);
  auto end_surface_0 = uvToCoord(intersection.points.back().surface0);
  auto end_surface_1 = uvToCoord(intersection.points.back().surface0);

  Segment curve_0 = {.start = start_surface_0, .end = end_surface_0};
  Segment curve_1 = {.start = start_surface_1, .end = end_surface_1};

  auto texture0 = std::make_unique<IntersectionTexture>(points0, bounds.first);
  auto texture1 = std::make_unique<IntersectionTexture>(points1, bounds.second);

  texture0->setIntersectionCurve({curve_0});
  texture1->setIntersectionCurve({curve_1});

  return {std::move(texture0), std::move(texture1)};
}

void IntersectionTexture::bind() const { texture_->bind(0); }

IntersectionTexture::IntersectionTexture(
    const std::vector<algebra::Vec2f> &surfacePoints,
    const std::array<algebra::Vec2f, 2> &bounds)
    : bounds_(bounds), canvas_(kWidth, kHeight) {
  texture_ = Texture::createTexture(kWidth, kHeight);
  fillCanvas(surfacePoints);
  texture_->fill(canvas_.getData());
}

uint32_t IntersectionTexture::getTextureId() const {
  return texture_->getTextureId();
}

void IntersectionTexture::drawLine(
    const std::vector<algebra::Vec2f> &surfacePoints, Color color) {
  const algebra::Vec2f u = bounds_[0];
  const algebra::Vec2f v = bounds_[1];

  auto toPixel = [&](const algebra::Vec2f &p) -> std::pair<int, int> {
    float normX = (p[0] - u[0]) / (u[1] - u[0]);
    float normY = (p[1] - v[0]) / (v[1] - v[0]);
    normX = std::max(normX, 0.001f);
    normY = std::max(normY, 0.001f);
    int x = std::clamp(static_cast<int>(normX * kWidth), 0, kWidth - 1);
    int y = std::clamp(static_cast<int>(normY * kHeight), 0, kHeight - 1);
    return {x, y};
  };

  auto drawPixel = [&](int x, int y) {
    int ix = std::clamp(x, 0, kWidth - 1);
    int iy = std::clamp(y, 0, kHeight - 1);
    int index = iy * kWidth + ix;
    canvas_.fillAtIndex(index, color);
  };

  for (size_t i = 1; i < surfacePoints.size(); ++i) {
    auto [x0, y0] = toPixel(surfacePoints[i - 1]);
    auto [x1, y1] = toPixel(surfacePoints[i]);

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    if (std::max(dx, dy) > 100 && surfacePoints.size() > 2) {
      continue;
    }

    int x = x0;
    int y = y0;
    while (true) {
      drawPixel(x, y);
      if (x == x1 && y == y1) {
        break;
      }

      int e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x += sx;
      }
      if (e2 < dx) {
        err += dx;
        y += sy;
      }
    }
  }
}

void IntersectionTexture::fillCanvas(
    const std::vector<algebra::Vec2f> &surfacePoints) {
  canvas_.fillWithColor(Color::Black());
  drawLine(surfacePoints);
}

void IntersectionTexture::floodFill(uint32_t x, uint32_t y, bool transparent) {

  std::vector<std::vector<bool>> visited(kHeight,
                                         std::vector<bool>(kWidth, false));

  auto isGreen = [&](uint32_t x, uint32_t y) {
    auto color = canvas_.colorAtIndex(y * kWidth + x);
    return color == Color::Green();
  };

  std::queue<std::pair<uint32_t, uint32_t>> q;
  q.emplace(x, y);
  visited[y][x] = true;

  const int dx[4] = {-1, 1, 0, 0};
  const int dy[4] = {0, 0, -1, 1};

  while (!q.empty()) {
    auto [x, y] = q.front();
    q.pop();

    if (transparent) {
      canvas_.fillAtIndex(y * kWidth + x, Color::Transparent());
    } else {
      canvas_.fillAtIndex(y * kWidth + x, Color::Black());
    }

    for (int d = 0; d < 4; ++d) {
      int nx = x + dx[d];
      int ny = y + dy[d];

      if (wrapU_) {
        if (nx < 0)
          nx += kWidth;
        if (nx >= static_cast<int>(kWidth))
          nx -= kWidth;
      }
      if (wrapV_) {
        if (ny < 0)
          ny += kHeight;
        if (ny >= static_cast<int>(kHeight))
          ny -= kHeight;
      }

      if (nx < 0 || nx >= static_cast<int>(kWidth) || ny < 0 ||
          ny >= static_cast<int>(kHeight)) {
        continue;
      }

      if (!visited[ny][nx] && !isGreen(nx, ny)) {
        visited[ny][nx] = true;
        q.emplace(nx, ny);
      }
    }
  }
  texture_->fill(canvas_.getData());
}

void IntersectionTexture::setCellType(uint32_t x, uint32_t y,
                                      CellType cellType) {
  Color color{};
  switch (cellType) {
  case CellType::Intersection:
    color = Color::Green();
    break;
  case CellType::Keep:
    color = Color::Black();
    break;
  case CellType::Trim:
    color = Color::Transparent();
    break;
  };

  canvas_.fillAtIndex(y * kWidth + x, color);
}

void IntersectionTexture::setColor(uint32_t x, uint32_t y, Color color) {
  canvas_.fillAtIndex(y * kWidth + x, color);
}

IntersectionTexture::CellType
IntersectionTexture::getCellType(uint32_t x, uint32_t y) const {
  auto color = canvas_.colorAtIndex(y * kWidth + x);
  if (color == Color::Green()) {
    return CellType::Intersection;
  }
  if (color == Color::Transparent()) {
    return CellType::Trim;
  }

  return CellType::Keep;
}

bool IntersectionTexture::isTrimmed(uint32_t x, uint32_t y) const {
  return getCellType(x, y) == CellType::Trim;
}

algebra::Vec2f IntersectionTexture::uv(uint32_t x, uint32_t y) const {
  float u = static_cast<float>(x) / static_cast<float>(kWidth);
  float v = static_cast<float>(y) / static_cast<float>(kHeight);
  return {u, v};
};

void IntersectionTexture::update() { texture_->fill(canvas_.getData()); }
IntersectionTexture::Coord

IntersectionTexture::uvToCoord(const algebra::Vec2f &uv) {
  auto x = static_cast<uint32_t>(uv.x() * static_cast<float>(kWidth));
  auto y = static_cast<uint32_t>(uv.y() * static_cast<float>(kHeight));

  return {.x = x, .y = y};
}

void IntersectionTexture::closeIntersectionCurve() {
  if (intersectionCurve_.size() < 2) {
    return;
  }

  auto first_segment = intersectionCurve_.front();
  auto last_segment = intersectionCurve_.back();
  auto start_uv = uv(first_segment.start.x, first_segment.start.y);
  auto end_uv = uv(last_segment.end.x, last_segment.end.y);

  drawLine({start_uv, end_uv});

  update();
}