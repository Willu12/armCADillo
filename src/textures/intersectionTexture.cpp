#include "intersectionTexture.hpp"
#include "color.hpp"
#include <queue>

std::pair<std::shared_ptr<IntersectionTexture>,
          std::shared_ptr<IntersectionTexture>>
IntersectionTexture::createIntersectionTextures(
    const Intersection &intersection,
    std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>
        bounds) {
  std::vector<algebra::Vec2f> points1(intersection.points.size());
  std::vector<algebra::Vec2f> points2(intersection.points.size());

  for (const auto &[i, p] :
       intersection.points | std::ranges::views::enumerate) {
    points1[i] = p.surface0;
    points2[i] = p.surface1;
  }

  auto texture1 = std::make_shared<IntersectionTexture>(points1, bounds.first);
  auto texture2 = std::make_shared<IntersectionTexture>(points2, bounds.second);
  return {std::move(texture1), std::move(texture2)};
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
    const std::vector<algebra::Vec2f> &surfacePoints) {
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
    canvas_.fillAtIndex(index, Color::Green());
  };

  for (size_t i = 1; i < surfacePoints.size(); ++i) {
    auto [x0, y0] = toPixel(surfacePoints[i - 1]);
    auto [x1, y1] = toPixel(surfacePoints[i]);

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    if (std::max(dx, dy) > 100) {
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
  visited[x][y] = true;

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

      // Handle wrapping if enabled
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
