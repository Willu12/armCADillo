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
  auto end_surface_1 = uvToCoord(intersection.points.back().surface1);

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

  const algebra::Vec2f u_bounds = bounds_[0];
  const algebra::Vec2f v_bounds = bounds_[1];

  // Helper: Convert UV to Pixel Coordinates
  auto toPixel = [&](const algebra::Vec2f &p) -> std::pair<int, int> {
    float normX = (p[0] - u_bounds[0]) / (u_bounds[1] - u_bounds[0]);
    float normY = (p[1] - v_bounds[0]) / (v_bounds[1] - v_bounds[0]);

    // Safety clamp (0.0 to 1.0)
    normX = std::clamp(normX, 0.0f, 1.0f);
    normY = std::clamp(normY, 0.0f, 1.0f);

    int x = std::clamp(static_cast<int>(normX * (kWidth - 1)), 0, kWidth - 1);
    int y = std::clamp(static_cast<int>(normY * (kHeight - 1)), 0, kHeight - 1);
    return {x, y};
  };

  // Helper: Actual Bresenham Algorithm (extracted to reuse)
  auto drawBresenham = [&](int x0, int y0, int x1, int y1) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    int x = x0;
    int y = y0;
    while (true) {
      // Draw pixel (bounds check built into fillAtIndex usually, but good to be
      // safe)
      if (x >= 0 && x < kWidth && y >= 0 && y < kHeight) {
        canvas_.fillAtIndex(y * kWidth + x, color);
      }

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
  };

  for (size_t i = 1; i < surfacePoints.size(); ++i) {
    const auto &p0 = surfacePoints[i - 1];
    const auto &p1 = surfacePoints[i];

    // Check Normalized UV distance to detect wrapping
    // Normalize U to 0..1 range for the check
    float u0_norm = (p0[0] - u_bounds[0]) / (u_bounds[1] - u_bounds[0]);
    float u1_norm = (p1[0] - u_bounds[0]) / (u_bounds[1] - u_bounds[0]);

    float u_diff = u1_norm - u0_norm;

    // THRESHOLD: If points are more than 50% of the texture width apart,
    // we assume they wrapped around the back.
    if (std::abs(u_diff) > 0.5f) {

      // --- WRAP DETECTED ---

      // Calculate the V height where the line hits the seam.
      // Total "logical" distance in U (accounting for wrap)
      // If wrapping Right (0.9 -> 0.1): dist = (1-0.9) + (0.1-0) = 0.2
      float logical_u_dist = (1.0f - std::abs(u_diff));

      // Slope of the line in V per unit of logical U
      float v_diff = p1[1] - p0[1];
      float slope = v_diff / logical_u_dist; // Check div/0 if needed

      // Calculate V at the boundary
      // Distance from P0 to its closest edge
      float dist_to_edge = (u_diff > 0) ? u0_norm : (1.0f - u0_norm);

      // Calculate intersection point Y value
      // Note: We need to normalize this delta math based on bounds if bounds
      // aren't 0-1 But assuming v_diff is in world space, this works:
      float boundary_v =
          p0[1] +
          (slope * dist_to_edge *
           (1.0f / (u_bounds[1] - u_bounds[0]))); // Scaling slope if needed

      // Simplified V-interp (Linear interpolation is usually fine for dense
      // points)
      float mid_v = (p0[1] + p1[1]) * 0.5f;

      // 1. Draw from P0 to Edge
      auto px0 = toPixel(p0);
      // If u_diff > 0 (0.1 -> 0.9), p0 is Left(0.1), closest edge is Left(0.0)
      // If u_diff < 0 (0.9 -> 0.1), p0 is Right(0.9), closest edge is
      // Right(1.0)

      int edge_x_1 = (u_diff < 0) ? (kWidth - 1) : 0;
      int edge_x_2 = (u_diff < 0) ? 0 : (kWidth - 1);

      auto edge_px_1 =
          toPixel({(u_diff < 0 ? u_bounds[1] : u_bounds[0]), mid_v});
      auto edge_px_2 =
          toPixel({(u_diff < 0 ? u_bounds[0] : u_bounds[1]), mid_v});

      // Draw First Segment (Start -> Edge)
      drawBresenham(px0.first, px0.second, edge_x_1, edge_px_1.second);

      // Draw Second Segment (Opposite Edge -> End)
      auto px1 = toPixel(p1);
      drawBresenham(edge_x_2, edge_px_2.second, px1.first, px1.second);

    } else {
      // --- NORMAL CASE (No Wrapping) ---
      auto [x0, y0] = toPixel(p0);
      auto [x1, y1] = toPixel(p1);
      drawBresenham(x0, y0, x1, y1);
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

  if (isGreen(x, y)) {
    return;
  }

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

      /*
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
    */
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
  return getCellType(x, y) != CellType::Keep;
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