#include "bezierSurfaceC0.hpp"
#include "bezierSurface.hpp"
#include "vec.hpp"
#include <array>
#include <memory>
#include <ranges>

std::vector<algebra::Vec3f>
BezierSurfaceC0::createFlatPositions(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float uLength, float vLength) {
  std::vector<algebra::Vec3f> controlPoints;
  const uint32_t u_points = 3 * uPatches + 1;
  const uint32_t v_points = 3 * vPatches + 1;

  controlPoints.reserve(u_points * v_points);
  for (uint32_t i = 0; i < v_points; ++i) {
    for (uint32_t j = 0; j < u_points; ++j) {
      controlPoints.emplace_back(
          position[0] + static_cast<float>(j) / 3.f * uLength,
          position[1] + static_cast<float>(i) / 3.f * vLength, position[2]);
    }
  }
  return controlPoints;
}

std::vector<algebra::Vec3f>
BezierSurfaceC0::createCyllinderPositions(const algebra::Vec3f &position,
                                          uint32_t uPatches, uint32_t vPatches,
                                          float r, float h) {
  const uint32_t u_points = 3 * uPatches + 1;
  const uint32_t v_points = 3 * vPatches + 1;
  std::vector<algebra::Vec3f> controlPoints;
  controlPoints.reserve(u_points * v_points);

  for (uint32_t i = 0; i < v_points - 1; ++i) {
    float u_ratio = static_cast<float>(i) / static_cast<float>(v_points);
    float angle = u_ratio * 2.0f * std::numbers::pi_v<float>;
    float x_circle = std::cos(angle) * r;
    float y_circle = std::sin(angle) * r;

    for (uint32_t j = 0; j < u_points; ++j) {
      float v_ratio = static_cast<float>(j) / static_cast<float>(u_points);
      float z = v_ratio * h;

      controlPoints.emplace_back(x_circle + position[0], y_circle + position[1],
                                 z + position[2]);
    }
  }
  return controlPoints;
}

BezierSurfaceC0::BezierSurfaceC0(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    uint32_t uCount, uint32_t vCount, bool cyllinder) {
  _id = kClassId++;
  _name = "BezierSurfaceC0_" + std::to_string(_id);

  for (const auto &controlPoint : points) {
    controlPoint.get().surfacePoint() = true;
    subscribe(controlPoint);
  }
  _points = points;
  if (cyllinder) {
    for (int i = 0; i < (3 * uCount + 1); ++i) {
      _points.push_back(_points[i]);
    }
    isCyllinder() = cyllinder;
  }
  _polyMesh = createPolyMesh();
  _patches = {.colCount = uCount, .rowCount = vCount};
  updateMesh();
}

std::unique_ptr<BezierSurfaceMesh> BezierSurfaceC0::generateMesh() {
  std::vector<float> controlPointsPositions(_points.size() * 3);

  for (const auto &[i, point] : _points | std::views::enumerate) {
    controlPointsPositions[3 * i] = point.get().getPosition()[0];
    controlPointsPositions[3 * i + 1] = point.get().getPosition()[1];
    controlPointsPositions[3 * i + 2] = point.get().getPosition()[2];
  }
  return BezierSurfaceMesh::create(controlPointsPositions, _patches.colCount,
                                   _patches.rowCount);
}

std::array<algebra::Vec2f, 2> BezierSurfaceC0::bounds() const {
  return {algebra::Vec2f{0.f, 1.f}, algebra::Vec2f{0.f, 1.f}};
}
algebra::Vec3f BezierSurfaceC0::value(const algebra::Vec2f &pos) const {
  auto patch = getCorrespondingBezierPatch(pos);
  const auto &P = patch.patch;
  const auto &uv = patch.localPos;

  algebra::Vec3f result{0.f, 0.f, 0.f};
  for (uint32_t i = 0; i < 4; ++i) {
    float Bu = bernstein(i, 3, uv[1]);
    for (uint32_t j = 0; j < 4; ++j) {
      float Bv = bernstein(j, 3, uv[0]);
      result = result + Bu * Bv * P[i][j];
    }
  }
  return result;
}
std::pair<algebra::Vec3f, algebra::Vec3f>
BezierSurfaceC0::derivatives(const algebra::Vec2f &pos) const {
  auto patch = getCorrespondingBezierPatch(pos);
  const auto &P = patch.patch;
  const auto &uv = patch.localPos;

  algebra::Vec3f dv{0.f, 0.f, 0.f}; // formerly 'du'
  algebra::Vec3f du{0.f, 0.f, 0.f}; // formerly 'dv'

  // ∂S/∂v — moving along i (rows)
  for (uint32_t i = 0; i < 3; ++i) {
    float Bv = bernstein(i, 2, uv[1]);
    for (uint32_t j = 0; j < 4; ++j) {
      float Bu = bernstein(j, 3, uv[0]);
      dv = dv + (P[i + 1][j] - P[i][j]) * (Bv * Bu);
    }
  }
  dv = dv * 3.f * _patches.rowCount;

  // ∂S/∂u — moving along j (columns)
  for (uint32_t i = 0; i < 4; ++i) {
    float Bv = bernstein(i, 3, uv[1]);
    for (uint32_t j = 0; j < 3; ++j) {
      float Bu = bernstein(j, 2, uv[0]);
      du = du + (P[i][j + 1] - P[i][j]) * (Bv * Bu);
    }
  }
  du = du * 3.f * _patches.colCount;

  return {du, dv};
}

algebra::Matrix<float, 3, 2>
BezierSurfaceC0::jacobian(const algebra::Vec2f &pos) const {
  auto [du, dv] = derivatives(pos);

  algebra::Matrix<float, 3, 2> J;
  J(0, 0) = du[0];
  J(1, 0) = du[1];
  J(2, 0) = du[2];

  J(0, 1) = dv[0];
  J(1, 1) = dv[1];
  J(2, 1) = dv[2];

  return J;
}

bool BezierSurfaceC0::wrapped(size_t dim) const {
  if (dim == 0)
    return isCyllinder();
  return false;
}

LocalBezierPatch
BezierSurfaceC0::getCorrespondingBezierPatch(const algebra::Vec2f &pos) const {
  // Clamp to just below 1 to avoid falling out of bounds
  float clampedU =
      std::min(pos[0], 1.0f - std::numeric_limits<float>::epsilon());
  float clampedV =
      std::min(pos[1], 1.0f - std::numeric_limits<float>::epsilon());

  // Compute which patch we're in
  uint32_t colPatchIndex =
      static_cast<uint32_t>(std::floor(clampedU * _patches.colCount));
  uint32_t rowPatchIndex =
      static_cast<uint32_t>(std::floor(clampedV * _patches.rowCount));

  // Compute local position within patch
  float patchUStart = static_cast<float>(colPatchIndex) / _patches.colCount;
  float patchVStart = static_cast<float>(rowPatchIndex) / _patches.rowCount;

  float localU = (clampedU - patchUStart) * _patches.colCount;
  float localV = (clampedV - patchVStart) * _patches.rowCount;

  // Extract 4x4 control points for the patch
  std::array<std::array<algebra::Vec3f, 4>, 4> patch;
  uint32_t u_offset = colPatchIndex * 3;
  uint32_t v_offset = rowPatchIndex * 3;
  uint32_t u_points = 3 * _patches.colCount + 1;

  for (uint32_t row = 0; row < 4; ++row) {
    for (uint32_t col = 0; col < 4; ++col) {
      uint32_t globalRow = v_offset + row;
      uint32_t globalCol = u_offset + col;
      patch[row][col] =
          _points[globalRow * u_points + globalCol].get().getPosition();
    }
  }

  return LocalBezierPatch{.patch = patch, .localPos = {localU, localV}};
}
