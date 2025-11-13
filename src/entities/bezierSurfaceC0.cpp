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
  std::vector<algebra::Vec3f> control_points;
  const uint32_t u_points = 3 * uPatches + 1;
  const uint32_t v_points = 3 * vPatches + 1;

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
BezierSurfaceC0::createCyllinderPositions(const algebra::Vec3f &position,
                                          uint32_t uPatches, uint32_t vPatches,
                                          float r, float h) {
  const uint32_t u_points = 3 * uPatches + 1;
  const uint32_t v_points = 3 * vPatches + 1;
  std::vector<algebra::Vec3f> control_points;
  control_points.reserve(u_points * v_points);
  for (uint32_t j = 0; j < v_points; ++j) { // v = vertical
    float v_ratio = static_cast<float>(j) / static_cast<float>(v_points - 1);
    float z = v_ratio * h;

    for (uint32_t i = 0; i < u_points; ++i) { // u = angle
      float u_ratio = static_cast<float>(i) / static_cast<float>(u_points - 1);
      float angle = u_ratio * 2.0f * std::numbers::pi_v<float>;
      float x = std::cos(angle) * r;
      float y = std::sin(angle) * r;

      control_points.emplace_back(position[0] + x, position[1] + y,
                                  position[2] + z);
    }
  }

  return control_points;
}

BezierSurfaceC0::BezierSurfaceC0(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    uint32_t uCount, uint32_t vCount, algebra::ConnectionType connectionType) {
  _id = kClassId++;
  _name = "BezierSurfaceC0_" + std::to_string(_id);
  _points = points;

  for (const auto &control_point : points) {
    control_point.get().surfacePoint() = true;
    subscribe(control_point);
  }

  _connectionType = connectionType;
  _polyMesh = createPolyMesh();
  _patches = {.colCount = uCount, .rowCount = vCount};
  update();
}

std::unique_ptr<BezierSurfaceMesh> BezierSurfaceC0::generateMesh() {
  std::vector<float> control_points_positions(_points.size() * 3);

  for (const auto &[i, point] : _points | std::views::enumerate) {
    control_points_positions[3 * i] = point.get().getPosition()[0];
    control_points_positions[3 * i + 1] = point.get().getPosition()[1];
    control_points_positions[3 * i + 2] = point.get().getPosition()[2];
  }
  return BezierSurfaceMesh::create(control_points_positions, _patches.colCount,
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