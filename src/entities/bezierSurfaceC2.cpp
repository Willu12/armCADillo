#include "bezierSurfaceC2.hpp"
#include "bezierSurface.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <array>
#include <cmath>
#include <memory>
#include <numbers>
#include <print>
#include <ranges>

void BezierSurfaceC2::updateBezierSurface() {
  _bezierControlPoints.clear();
  _rowOrderBezierControlPoints.clear();
  const uint32_t col_patches = _patches.colCount;
  const uint32_t row_patches = _patches.rowCount;
  const uint32_t bezier_cols = 3 * col_patches + 1;
  _rowOrderBezierControlPoints.resize((3 * row_patches + 1) * bezier_cols);
  const uint32_t col_deboor_points = 3 + col_patches;

  for (uint32_t row_patch = 0; row_patch < row_patches; ++row_patch) {
    for (uint32_t col_patch = 0; col_patch < col_patches; ++col_patch) {

      std::array<std::array<algebra::Vec3f, 4>, 4> patch;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          patch[i][j] =
              _points[(row_patch + i) * col_deboor_points + col_patch + j]
                  .get()
                  .getPosition();
        }
      }

      auto final_patch = processPatch(patch);

      for (const auto &[rowCount, row] : final_patch | std::views::enumerate) {
        for (const auto &[colCount, col] : row | std::views::enumerate) {
          const uint32_t global_row = row_patch * 3 + rowCount;
          const uint32_t global_col = col_patch * 3 + colCount;
          const uint32_t global_index =
              global_row * (3 * col_patches + 1) + global_col;
          _rowOrderBezierControlPoints[global_index] =
              final_patch[rowCount][colCount];
          _bezierControlPoints.emplace_back(col);
        }
      }
    }
  }
  updateAlgebraicSurfaceC0();
}

std::unique_ptr<BezierSurfaceMesh> BezierSurfaceC2::generateMesh() {
  std::vector<float> control_points_positions(_bezierControlPoints.size() * 3);

  for (const auto &[i, point] : _bezierControlPoints | std::views::enumerate) {
    control_points_positions[3 * i] = point[0];
    control_points_positions[3 * i + 1] = point[1];
    control_points_positions[3 * i + 2] = point[2];
  }
  return BezierSurfaceMesh::createC2(control_points_positions,
                                     _patches.colCount, _patches.rowCount);
}

BezierSurfaceC2::BezierSurfaceC2(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    uint32_t uCount, uint32_t vCount, algebra::ConnectionType connectionType) {
  _id = kClassId++;
  _name = "BezierSurfaceC2_" + std::to_string(_id);
  _points = points;
  for (const auto &point : points) {
    point.get().surfacePoint() = true;
    subscribe(point);
  }
  _polyMesh = createPolyMesh();
  _patches = {.colCount = uCount, .rowCount = vCount};
  _connectionType = connectionType;
  updateBezierSurface();
  update();
}

std::vector<algebra::Vec3f>
BezierSurfaceC2::createFlatPositions(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float uLength, float vLength) {

  std::vector<algebra::Vec3f> control_points;
  const uint32_t u_points = 3 + uPatches;
  const uint32_t v_points = 3 + vPatches;

  control_points.reserve(u_points * v_points);
  for (uint32_t i = 0; i < v_points; ++i) {
    for (uint32_t j = 0; j < u_points; ++j) {
      control_points.emplace_back(
          position[0] + static_cast<float>(j) / 3.f * uLength,
          position[1] + static_cast<float>(i) / 3.f * vLength, position[2]);
    }
  }
  return control_points;
}

std::vector<algebra::Vec3f>
BezierSurfaceC2::createCyllinderPositions(const algebra::Vec3f &position,
                                          uint32_t uPatches, uint32_t vPatches,
                                          float r, float h) {
  const uint32_t u_points = 3 + uPatches;
  const uint32_t v_points = 3 + vPatches;
  std::vector<algebra::Vec3f> control_points;

  control_points.reserve(u_points * v_points);
  const float angle =
      2.0f * std::numbers::pi_v<float> / static_cast<float>(u_points - 3);
  float new_r = 3 * r / (std::cos(angle) + 2);

  for (uint32_t i = 0; i < v_points; ++i) {
    float v_ration = static_cast<float>(i) / static_cast<float>(v_points - 1);
    float z = v_ration * h;

    for (uint32_t j = 0; j < u_points - 3; ++j) {
      float u_ratio = static_cast<float>(j) / static_cast<float>(u_points - 3);
      float current_angle = u_ratio * 2.0f * std::numbers::pi_v<float>;

      float x_circle = std::cos(current_angle) * new_r;
      float y_circle = std::sin(current_angle) * new_r;
      control_points.emplace_back(x_circle + position[0],
                                  y_circle + position[1], z + position[2]);
    }
  }
  return control_points;
}

std::array<std::array<algebra::Vec3f, 4>, 4> BezierSurfaceC2::processPatch(
    const std::array<std::array<algebra::Vec3f, 4>, 4> &patch) {
  algebra::Vec3f row_converted[4][4];
  for (int i = 0; i < 4; ++i) {
    const algebra::Vec3f &d0 = patch[i][0];
    const algebra::Vec3f &d1 = patch[i][1];
    const algebra::Vec3f &d2 = patch[i][2];
    const algebra::Vec3f &d3 = patch[i][3];

    row_converted[i][0] = (d0 + 4.f * d1 + d2) / 6.f;
    row_converted[i][1] = (4.f * d1 + 2.f * d2) / 6.f;
    row_converted[i][2] = (2.f * d1 + 4.f * d2) / 6.f;
    row_converted[i][3] = (d1 + 4.f * d2 + d3) / 6.f;
  }
  std::array<std::array<algebra::Vec3f, 4>, 4> final_patch;
  for (int j = 0; j < 4; ++j) {
    const algebra::Vec3f &d0 = row_converted[0][j];
    const algebra::Vec3f &d1 = row_converted[1][j];
    const algebra::Vec3f &d2 = row_converted[2][j];
    const algebra::Vec3f &d3 = row_converted[3][j];

    final_patch[0][j] = (d0 + 4.f * d1 + d2) / 6.f;
    final_patch[1][j] = (4.f * d1 + 2.f * d2) / 6.f;
    final_patch[2][j] = (2.f * d1 + 4.f * d2) / 6.f;
    final_patch[3][j] = (d1 + 4.f * d2 + d3) / 6.f;
  }
  return final_patch;
}

std::vector<algebra::Vec3f> BezierSurfaceC2::getRowOrderedBezierPoints() const {
  return _rowOrderBezierControlPoints;
}
void BezierSurfaceC2::updateAlgebraicSurfaceC0() {
  const auto &bezier_points = getRowOrderedBezierPoints();

  _algebraSurfaceC0 = std::make_unique<algebra::BezierSurfaceC0>(
      bezier_points, _patches.colCount, _patches.rowCount, _connectionType);
}