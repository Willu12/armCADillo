#include "bezierSurfaceC0.hpp"
#include <memory>

std::shared_ptr<BezierSurfaceC0>
BezierSurfaceC0::createFlat(const algebra::Vec3f &position, uint32_t u_patches,
                            uint32_t v_patches) {
  std::vector<algebra::Vec3f> controlPoints;

  const float length = 1.0f;
  const uint32_t u_points = 3 * u_patches + 1;
  const uint32_t v_points = 3 * v_patches + 1;

  controlPoints.reserve(u_points * v_points);
  for (uint32_t i = 0; i < u_points; ++i) {
    for (uint32_t j = 0; j < v_points; ++j) {
      controlPoints.emplace_back(position[0] + float(i) / 3.f * length,
                                 position[1] + float(j) / 3.f * length,
                                 position[2]);
    }
  }

  auto flatSurface = new BezierSurfaceC0(controlPoints);
  flatSurface->_patches.sCount = u_patches;
  flatSurface->_patches.tCount = v_patches;
  flatSurface->_mesh = flatSurface->generateMesh();

  return std::shared_ptr<BezierSurfaceC0>(flatSurface);
}

std::shared_ptr<BezierSurfaceC0>
BezierSurfaceC0::createCylinder(const algebra::Vec3f &position, float r,
                                float h) {
  const uint32_t u_patches = h / 0.5f;
  const uint32_t v_patches = 2;

  const uint32_t u_points = 3 * u_patches + 1;
  const uint32_t v_points = 3 * v_patches + 1;

  std::vector<algebra::Vec3f> controlPoints;
  controlPoints.reserve(u_points * v_points);

  for (uint32_t i = 0; i < u_points; ++i) {
    float u_ratio = float(i) / float(3 * u_patches);
    float angle = u_ratio * 2.0f * M_PI;

    float x_circle = std::cos(angle) * r;
    float y_circle = std::sin(angle) * r;

    for (uint32_t j = 0; j < v_points; ++j) {
      float v_ratio = float(j) / float(3 * v_patches);
      float z = v_ratio * h;

      controlPoints.emplace_back(x_circle + position[0], y_circle + position[1],
                                 z + position[2]);
    }
  }
  auto cylinderSurface = new BezierSurfaceC0(controlPoints);

  cylinderSurface->_patches.sCount = u_patches;
  cylinderSurface->_patches.tCount = v_patches;
  cylinderSurface->_mesh = cylinderSurface->generateMesh();

  return std::shared_ptr<BezierSurfaceC0>(cylinderSurface);
}

BezierSurfaceC0::BezierSurfaceC0(const std::vector<algebra::Vec3f> &positions) {
  _name = "BezierCurveC0_" + std::to_string(_id++);

  for (const auto &controlPoint : positions) {
    auto point = std::make_unique<PointEntity>(controlPoint);
    point->surfacePoint() = true;
    subscribe(*point);
    _points.emplace_back(std::move(point));
  }
}
