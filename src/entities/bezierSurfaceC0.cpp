#include "bezierSurfaceC0.hpp"
#include <memory>

std::shared_ptr<BezierSurfaceC0>
BezierSurfaceC0::createFlat(const algebra::Vec3f &position, uint32_t u_patches,
                            uint32_t v_patches) {
  auto flatSurface =
      new BezierSurfaceC0(createFlatPoints(position, u_patches, v_patches));
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

  auto cylinderSurface =
      new BezierSurfaceC0(createCylinderPoints(position, r, h));

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

std::unique_ptr<BezierSurfaceMesh> BezierSurfaceC0::generateMesh() {
  std::vector<float> controlPointsPositions(_points.size() * 3);

  for (const auto &[i, point] : _points | std::views::enumerate) {
    controlPointsPositions[3 * i] = point->getPosition()[0];
    controlPointsPositions[3 * i + 1] = point->getPosition()[1];
    controlPointsPositions[3 * i + 2] = point->getPosition()[2];
  }
  return BezierSurfaceMesh::create(controlPointsPositions, _patches.sCount,
                                   _patches.tCount);
}