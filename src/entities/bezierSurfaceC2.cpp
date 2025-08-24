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
  const uint32_t colPatches = _patches.colCount;
  const uint32_t rowPatches = _patches.rowCount;
  const uint32_t bezierCols = 3 * colPatches + 1;
  _rowOrderBezierControlPoints.resize((3 * rowPatches + 1) * bezierCols);
  const uint32_t colDeboorPoints = 3 + colPatches;

  for (uint32_t rowPatch = 0; rowPatch < rowPatches; ++rowPatch) {
    for (uint32_t colPatch = 0; colPatch < colPatches; ++colPatch) {

      std::array<std::array<algebra::Vec3f, 4>, 4> patch;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          patch[i][j] = _points[(rowPatch + i) * colDeboorPoints + colPatch + j]
                            .get()
                            .getPosition();
        }
      }

      auto finalPatch = processPatch(patch);

      for (const auto &[rowCount, row] : finalPatch | std::views::enumerate) {
        for (const auto &[colCount, col] : row | std::views::enumerate) {
          const uint32_t globalRow = rowPatch * 3 + rowCount;
          const uint32_t globalCol = colPatch * 3 + colCount;
          const uint32_t globalIndex =
              globalRow * (3 * colPatches + 1) + globalCol;
          _rowOrderBezierControlPoints[globalIndex] =
              finalPatch[rowCount][colCount];
          _bezierControlPoints.emplace_back(col);
        }
      }
    }
  }
  updateAlgebraicSurfaceC0();
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
    uint32_t uCount, uint32_t vCount, algebra::ConnectionType connectionType) {
  _id = kClassId++;
  _name = "BezierCurveC2_" + std::to_string(_id);

  for (const auto &point : points) {
    point.get().surfacePoint() = true;
    subscribe(point);
  }
  _points = points;
  _polyMesh = createPolyMesh();
  _patches = {.colCount = vCount, .rowCount = uCount};
  _connectionType = connectionType;
  updateBezierSurface();
  update();
}

std::vector<algebra::Vec3f>
BezierSurfaceC2::createFlatPositions(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float uLength, float vLength) {

  std::vector<algebra::Vec3f> controlPoints;
  const uint32_t uPoints = 3 + uPatches;
  const uint32_t vPoints = 3 + vPatches;

  controlPoints.reserve(uPoints * vPoints);
  for (uint32_t i = 0; i < uPoints; ++i) {
    for (uint32_t j = 0; j < vPoints; ++j) {
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
  const uint32_t uPoints = 3 + uPatches;
  const uint32_t vPoints = 3 + vPatches;
  std::vector<algebra::Vec3f> controlPoints;

  controlPoints.reserve(uPoints * vPoints);
  const float angleBetweenPoints =
      1.f / static_cast<float>(uPoints - 3) * 2.0f * std::numbers::pi_v<float>;
  float newR = 3 * r / (std::cos(angleBetweenPoints) + 2);

  for (uint32_t i = 0; i < uPoints; ++i) {
    float uRatio = static_cast<float>(i) / static_cast<float>(uPoints - 3);
    float angle = uRatio * 2.0f * std::numbers::pi_v<float>;

    float xCircle = std::cos(angle) * newR;
    float yCircle = std::sin(angle) * newR;

    for (uint32_t j = 0; j < vPoints; ++j) {
      float vRatio = static_cast<float>(j) / static_cast<float>(vPoints - 1);
      float z = vRatio * h;

      controlPoints.emplace_back(xCircle + position[0], yCircle + position[1],
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

std::vector<algebra::Vec3f> BezierSurfaceC2::getRowOrderedBezierPoints() const {
  return _rowOrderBezierControlPoints;
}
void BezierSurfaceC2::updateAlgebraicSurfaceC0() {
  const auto &bezierPoints = getRowOrderedBezierPoints();

  _algebraSurfaceC0 = std::make_unique<algebra::BezierSurfaceC0>(
      bezierPoints, _patches.colCount, _patches.rowCount, _connectionType);
}