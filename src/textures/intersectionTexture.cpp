#include "intersectionTexture.hpp"
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
  // canvas_.flipY();
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
    canvas_.fillAtIndex(index, {0, 255, 0, 255});
  };

  for (size_t i = 1; i < surfacePoints.size(); ++i) {
    auto [x0, y0] = toPixel(surfacePoints[i - 1]);
    auto [x1, y1] = toPixel(surfacePoints[i]);

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    // Optionally skip long segments (same logic as before)
    if (std::max(dx, dy) > 100)
      continue;

    int x = x0;
    int y = y0;
    while (true) {
      drawPixel(x, y);
      if (x == x1 && y == y1)
        break;

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

std::vector<std::pair<uint32_t, uint32_t>>
IntersectionTexture::findInteriorSamplePoints() {
  std::vector<std::pair<uint32_t, uint32_t>> interiorSamples;

  std::vector<std::vector<bool>> visited(kHeight,
                                         std::vector<bool>(kWidth, false));
  std::vector<std::vector<bool>> isInterior(kHeight,
                                            std::vector<bool>(kWidth, false));
  std::vector<std::vector<bool>> isBackground(kHeight,
                                              std::vector<bool>(kWidth, false));

  auto isGreen = [&](uint32_t x, uint32_t y) {
    auto color = canvas_.colorAtIndex(y * kWidth + x);
    return color == Color{0, 255, 0, 255};
  };

  // Step 1: Mark interior pixels between green edges using scanline
  for (uint32_t y = 0; y < kHeight; ++y) {
    std::vector<uint32_t> crossings;

    for (uint32_t x = 0; x < kWidth; ++x) {
      if (isGreen(x, y)) {
        crossings.push_back(x);
      }
    }

    if (crossings.size() < 2)
      continue;

    for (size_t i = 0; i + 1 < crossings.size(); i += 2) {
      for (uint32_t x = crossings[i] + 1; x < crossings[i + 1]; ++x) {
        isInterior[y][x] = true;
      }
    }
  }

  // Step 2: Flood-fill from (0, 0) to mark background
  std::queue<std::pair<uint32_t, uint32_t>> q;
  q.emplace(0, 0);
  isBackground[0][0] = true;

  const int dx[] = {-1, 1, 0, 0};
  const int dy[] = {0, 0, -1, 1};

  while (!q.empty()) {
    auto [x, y] = q.front();
    q.pop();

    for (int d = 0; d < 4; ++d) {
      int nx = x + dx[d];
      int ny = y + dy[d];
      if (nx >= 0 && nx < static_cast<int>(kWidth) && ny >= 0 &&
          ny < static_cast<int>(kHeight) && !isGreen(nx, ny) &&
          !isInterior[ny][nx] && !isBackground[ny][nx]) {
        isBackground[ny][nx] = true;
        q.emplace(nx, ny);
      }
    }
  }

  // Step 3: Find interior regions that are not background
  auto markRegion = [&](uint32_t sx, uint32_t sy) {
    std::queue<std::pair<uint32_t, uint32_t>> q;
    q.emplace(sx, sy);
    visited[sy][sx] = true;

    while (!q.empty()) {
      auto [x, y] = q.front();
      q.pop();

      for (int d = 0; d < 4; ++d) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (nx >= 0 && nx < static_cast<int>(kWidth) && ny >= 0 &&
            ny < static_cast<int>(kHeight) && isInterior[ny][nx] &&
            !visited[ny][nx]) {
          visited[ny][nx] = true;
          q.emplace(nx, ny);
        }
      }
    }
  };

  for (uint32_t y = 0; y < kHeight; ++y) {
    for (uint32_t x = 0; x < kWidth; ++x) {
      if (isInterior[y][x] && !visited[y][x] && !isBackground[y][x]) {
        interiorSamples.emplace_back(x, y); // One sample per shape
        markRegion(x, y); // Mark region to skip further sampling
      }
    }
  }

  return interiorSamples;
}

std::vector<std::vector<algebra::Vec2f>>
IntersectionTexture::divideSurfacePoints(
    const std::vector<algebra::Vec2f> &surfacePoints) {
  const float epsilon = 0.01f;

  std::vector<std::vector<algebra::Vec2f>> dividedPoints;
  if (surfacePoints.empty())
    return dividedPoints;

  std::vector<algebra::Vec2f> currentSegment;
  currentSegment.push_back(surfacePoints[0]);

  for (size_t i = 1; i < surfacePoints.size(); ++i) {
    const auto &prev = surfacePoints[i - 1];
    const auto &curr = surfacePoints[i];
    float dist = (curr - prev).length();

    if (dist > epsilon) {
      dividedPoints.push_back(currentSegment);
      currentSegment.clear();
    }
    currentSegment.push_back(curr);
  }

  // Push the last segment
  if (!currentSegment.empty())
    dividedPoints.push_back(currentSegment);

  return dividedPoints;
}

void IntersectionTexture::fillCanvas(
    const std::vector<algebra::Vec2f> &surfacePoints) {
  canvas_.fillWithColor({0, 0, 0, 255});

  // auto dividedPoints = divideSurfacePoints(surfacePoints);
  //  for (const auto &points : dividedPoints)
  drawLine(surfacePoints);

  // drawLine(surfacePoints);

  auto interior = findInteriorSamplePoints();
  if (interior.size() == 0)
    floodFillTransparent(0, 0);
  else
    for (const auto &inter : interior)
      floodFillTransparent(inter.first, inter.second);

  //    floodFillTransparent(interior->first, interior->second);
  //  floodFillTransparent(50, 0);
  // floodFillTransparent(50, 280);
}

void IntersectionTexture::floodFillTransparent(int startX, int startY) {
  std::queue<std::pair<int, int>> q;
  std::vector<bool> visited(kWidth * kHeight, false);

  q.emplace(startX, startY);
  visited[startY * kWidth + startX] = true;

  while (!q.empty()) {
    auto [x, y] = q.front();
    q.pop();

    int index = (y * kWidth + x);
    // If pixel is black (curve), don't fill past it
    if (canvas_.colorAtIndex(index) == Color{0, 255, 0, 255})
      continue;

    // Make transparent
    canvas_.fillAtIndex(index, {0, 0, 0, 0});

    // Explore neighbors
    const std::pair<int, int> neighbors[4] = {
        {x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}};
    for (auto [nx, ny] : neighbors) {
      if (nx >= 0 && nx < kWidth && ny >= 0 && ny < kHeight) {
        int nidx = ny * kWidth + nx;
        if (!visited[nidx]) {
          visited[nidx] = true;
          q.emplace(nx, ny);
        }
      }
    }
  }
}