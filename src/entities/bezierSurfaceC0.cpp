#include "bezierSurfaceC0.hpp"
#include "bezierSurface.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <memory>
#include <ranges>

std::vector<algebra::Vec3f>
BezierSurfaceC0::createFlatPositions(const algebra::Vec3f &position,
                                     uint32_t uPatches, uint32_t vPatches,
                                     float uLength, float vLength) {
  std::vector<algebra::Vec3f> controlPoints;
  const uint32_t uPoints = 3 * uPatches + 1;
  const uint32_t vPoints = 3 * vPatches + 1;

  controlPoints.reserve(uPoints * vPoints);
  for (uint32_t i = 0; i < vPoints; ++i) {
    for (uint32_t j = 0; j < uPoints; ++j) {
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
  const uint32_t uPoints = 3 * uPatches + 1;
  const uint32_t vPoints = 3 * vPatches + 1;
  std::vector<algebra::Vec3f> controlPoints;
  controlPoints.reserve(uPoints * vPoints);

  for (uint32_t i = 0; i < vPoints - 1; ++i) {
    float uRatio = static_cast<float>(i) / static_cast<float>(vPoints);
    float angle = uRatio * 2.0f * std::numbers::pi_v<float>;
    float xCircle = std::cos(angle) * r;
    float yCircle = std::sin(angle) * r;

    for (uint32_t j = 0; j < uPoints; ++j) {
      float vRatio = static_cast<float>(j) / static_cast<float>(uPoints);
      float z = vRatio * h;

      controlPoints.emplace_back(xCircle + position[0], yCircle + position[1],
                                 z + position[2]);
    }
  }
  return controlPoints;
}

BezierSurfaceC0::BezierSurfaceC0(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    uint32_t uCount, uint32_t vCount, algebra::ConnectionType connectionType) {
  _id = kClassId++;
  _name = "BezierSurfaceC0_" + std::to_string(_id);

  for (const auto &controlPoint : points) {
    controlPoint.get().surfacePoint() = true;
    subscribe(controlPoint);
  }
  _points = points;
  if (connectionType == algebra::ConnectionType::Columns) {
    for (int i = 0; i < (3 * uCount + 1); ++i) {
      _points.push_back(_points[i]);
    }
    _connectionType = algebra::ConnectionType::Columns;
  }
  _polyMesh = createPolyMesh();
  _patches = {.colCount = uCount, .rowCount = vCount};
  update();
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

void BezierSurfaceC0::updateAlgebraicSurfaceC0() {
  std::vector<algebra::Vec3f> points(_points.size());
  for (const auto &[i, p] : _points | std::views::enumerate) {
    points[i] = p.get().getPosition();
  }
  _algebraSurfaceC0 = std::make_unique<algebra::BezierSurfaceC0>(
      points, _patches.colCount, _patches.rowCount, _connectionType);
}