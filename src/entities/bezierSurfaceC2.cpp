#include "bezierSurfaceC2.hpp"
#include "bezierSurface.hpp"
#include "vec.hpp"
#include <array>
#include <cmath>
#include <numbers>
#include <ranges>
#include <stdexcept>

void BezierSurfaceC2::updateBezierSurface() {
  _bezierControlPoints.clear();

  const uint32_t uPatches = _patches.colCount;
  const uint32_t vPatches = _patches.rowCount;
  const uint32_t vPoints = 3 + vPatches;

  for (uint32_t uPatch = 0; uPatch < uPatches; ++uPatch) {
    for (uint32_t vPatch = 0; vPatch < vPatches; ++vPatch) {

      std::array<std::array<algebra::Vec3f, 4>, 4> patch;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          patch[i][j] =
              _points[(uPatch + i) * vPoints + vPatch + j].get().getPosition();
        }
      }

      auto finalPatch = processPatch(patch);

      for (const auto &row : finalPatch) {
        for (const auto &j : row) {
          _bezierControlPoints.emplace_back(j);
        }
      }
    }
  }
}

std::unique_ptr<BezierSurfaceMesh> BezierSurfaceC2::generateMesh() {
  std::vector<float> controlPointsPositions(_bezierControlPoints.size() * 3);

  for (const auto &[i, point] : _bezierControlPoints | std::views::enumerate) {
    controlPointsPositions[3 * i] = point[0];
    controlPointsPositions[3 * i + 1] = point[1];
    controlPointsPositions[3 * i + 2] = point[2];
  }
  return BezierSurfaceMesh::createC2(controlPointsPositions, _patches.colCount,
                                     _patches.rowCount);
}

BezierSurfaceC2::BezierSurfaceC2(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    uint32_t uCount, uint32_t vCount) {
  _id = kClassId++;
  _name = "BezierCurveC2_" + std::to_string(_id);

  for (const auto &point : points) {
    point.get().surfacePoint() = true;
    subscribe(point);
  }
  _points = points;
  _polyMesh = createPolyMesh();
  _patches = {.colCount = uCount, .rowCount = vCount};
  updateMesh();
}

std::vector<algebra::Vec3f>
BezierSurfaceC2::createFlatPositions(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float uLength, float vLength) {

  std::vector<algebra::Vec3f> controlPoints;
  const uint32_t u_points = 3 + uPatches;
  const uint32_t v_points = 3 + vPatches;

  controlPoints.reserve(u_points * v_points);
  for (uint32_t i = 0; i < u_points; ++i) {
    for (uint32_t j = 0; j < v_points; ++j) {
      controlPoints.emplace_back(
          position[0] + static_cast<float>(j) / 3.f * uLength,
          position[1] + static_cast<float>(i) / 3.f * vLength, position[2]);
    }
  }
  return controlPoints;
}

std::vector<algebra::Vec3f>
BezierSurfaceC2::createCyllinderPositions(const algebra::Vec3f &position,
                                          uint32_t uPatches, uint32_t vPatches,
                                          float r, float h) {
  const uint32_t u_points = 3 + uPatches;
  const uint32_t v_points = 3 + vPatches;
  std::vector<algebra::Vec3f> controlPoints;

  controlPoints.reserve(u_points * v_points);
  const float angleBetweenPoints =
      1.f / static_cast<float>(u_points - 3) * 2.0f * std::numbers::pi_v<float>;
  float newR = 3 * r / (std::cos(angleBetweenPoints) + 2);

  for (uint32_t i = 0; i < u_points; ++i) {
    float u_ratio = static_cast<float>(i) / static_cast<float>(u_points - 3);
    float angle = u_ratio * 2.0f * std::numbers::pi_v<float>;

    float x_circle = std::cos(angle) * newR;
    float y_circle = std::sin(angle) * newR;

    for (uint32_t j = 0; j < v_points; ++j) {
      float v_ratio = static_cast<float>(j) / static_cast<float>(v_points - 1);
      float z = v_ratio * h;

      controlPoints.emplace_back(x_circle + position[0], y_circle + position[1],
                                 z + position[2]);
    }
  }
  return controlPoints;
}

std::array<std::array<algebra::Vec3f, 4>, 4> BezierSurfaceC2::processPatch(
    const std::array<std::array<algebra::Vec3f, 4>, 4> &patch) {
  algebra::Vec3f rowConverted[4][4];
  for (int i = 0; i < 4; ++i) {
    const algebra::Vec3f &d0 = patch[i][0];
    const algebra::Vec3f &d1 = patch[i][1];
    const algebra::Vec3f &d2 = patch[i][2];
    const algebra::Vec3f &d3 = patch[i][3];

    rowConverted[i][0] = (d0 + 4.f * d1 + d2) / 6.f;
    rowConverted[i][1] = (4.f * d1 + 2.f * d2) / 6.f;
    rowConverted[i][2] = (2.f * d1 + 4.f * d2) / 6.f;
    rowConverted[i][3] = (d1 + 4.f * d2 + d3) / 6.f;
  }
  std::array<std::array<algebra::Vec3f, 4>, 4> finalPatch;
  for (int j = 0; j < 4; ++j) {
    const algebra::Vec3f &d0 = rowConverted[0][j];
    const algebra::Vec3f &d1 = rowConverted[1][j];
    const algebra::Vec3f &d2 = rowConverted[2][j];
    const algebra::Vec3f &d3 = rowConverted[3][j];

    finalPatch[0][j] = (d0 + 4.f * d1 + d2) / 6.f;
    finalPatch[1][j] = (4.f * d1 + 2.f * d2) / 6.f;
    finalPatch[2][j] = (2.f * d1 + 4.f * d2) / 6.f;
    finalPatch[3][j] = (d1 + 4.f * d2 + d3) / 6.f;
  }
  return finalPatch;
}

bool BezierSurfaceC2::wrapped(size_t dim) const { return false; }
std::array<algebra::Vec2f, 2> BezierSurfaceC2::bounds() const {
  return {algebra::Vec2f{0.f, 1.f}, algebra::Vec2f{0.f, 1.f}};
}
algebra::Vec3f BezierSurfaceC2::value(const algebra::Vec2f &pos) const {
  const uint32_t u_points = 3 * _patches.rowCount + 1;
  const uint32_t v_points = 3 * _patches.colCount + 1;

  const uint32_t n = u_points - 1;
  const uint32_t m = v_points - 1;
  const auto bezierPoints = getRowOrderedBezierPoints();
  algebra::Vec3f result{0.f, 0.f, 0.f};
  for (uint32_t i = 0; i <= n; ++i) {
    float Bu = BezierSurface::bernstein(i, n, pos[0]);
    for (uint32_t j = 0; j <= m; ++j) {
      float Bv = BezierSurface::bernstein(j, m, pos[1]);
      const auto &Pij = bezierPoints[i * v_points + j];
      result = result + Bu * Bv * Pij;
    }
  }
  return result;
}
std::pair<algebra::Vec3f, algebra::Vec3f>
BezierSurfaceC2::derivatives(const algebra::Vec2f &pos) const {
  const uint32_t u_points = 3 * _patches.rowCount + 1;
  const uint32_t v_points = 3 * _patches.colCount + 1;

  const uint32_t n = u_points - 1;
  const uint32_t m = v_points - 1;

  algebra::Vec3f du{0.f, 0.f, 0.f};
  algebra::Vec3f dv{0.f, 0.f, 0.f};
  const auto bezierPoints = getRowOrderedBezierPoints();

  for (uint32_t i = 0; i < n; ++i) {
    float Bu = BezierSurface::bernstein(i, n - 1, pos[0]);
    for (uint32_t j = 0; j <= m; ++j) {
      float Bv = BezierSurface::bernstein(j, m, pos[1]);

      const algebra::Vec3f &P1 = bezierPoints[(i + 1) * v_points + j];
      const algebra::Vec3f &P0 = bezierPoints[i * v_points + j];
      du = du + (P1 - P0) * (Bu * Bv);
    }
  }
  du = du * static_cast<float>(n);

  for (uint32_t i = 0; i <= n; ++i) {
    float Bu = BezierSurface::bernstein(i, n, pos[0]);
    for (uint32_t j = 0; j < m; ++j) {
      float Bv = BezierSurface::bernstein(j, m - 1, pos[1]);

      const algebra::Vec3f &P1 = bezierPoints[i * v_points + (j + 1)];
      const algebra::Vec3f &P0 = bezierPoints[i * v_points + j];
      dv = dv + (P1 - P0) * (Bu * Bv);
    }
  }
  dv = dv * static_cast<float>(m);

  return {du, dv};
}

algebra::Matrix<float, 3, 2>
BezierSurfaceC2::jacobian(const algebra::Vec2f &pos) const {
  auto [du, dv] = derivatives(pos);

  algebra::Matrix<float, 3, 2> J;
  J(0, 0) = du[0];
  J(1, 0) = du[1];
  J(2, 0) = du[2];

  J(0, 1) = dv[0];
  J(1, 2) = dv[1];
  J(2, 3) = dv[2];

  return J;
}

std::vector<algebra::Vec3f> BezierSurfaceC2::getRowOrderedBezierPoints() const {
  const uint32_t uPatches = _patches.colCount;
  const uint32_t vPatches = _patches.rowCount;

  const uint32_t rows = 3 * vPatches + 1;
  const uint32_t cols = 3 * uPatches + 1;

  std::vector<algebra::Vec3f> grid(rows * cols);

  for (uint32_t u = 0; u < uPatches; ++u) {
    for (uint32_t v = 0; v < vPatches; ++v) {
      uint32_t patchIndex = u * vPatches + v;

      for (uint32_t i = 0; i < 4; ++i) {
        for (uint32_t j = 0; j < 4; ++j) {
          uint32_t localIndex = i * 4 + j;

          uint32_t row = 3 * v + i;
          uint32_t col = 3 * u + j;

          if (row < rows && col < cols) {
            grid[row * cols + col] =
                _bezierControlPoints[patchIndex * 16 + localIndex];
          }
        }
      }
    }
  }

  return grid;
}