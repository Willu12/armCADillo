#include "bezierSurfaceC2.hpp"
#include "vec.hpp"
#include <array>

void BezierSurfaceC2::updateBezierSurface() {
  _bezierControlPoints.clear();

  const uint32_t uPatches = _patches.sCount;
  const uint32_t vPatches = _patches.tCount;
  const uint32_t uPoints = 3 + uPatches;
  const uint32_t vPoints = 3 + vPatches;

  for (uint32_t uPatch = 0; uPatch < uPatches; ++uPatch) {
    for (uint32_t vPatch = 0; vPatch < vPatches; ++vPatch) {

      std::array<std::array<algebra::Vec3f, 4>, 4> patch;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          patch[i][j] =
              _points[(uPatch + i) * vPoints + vPatch + j]->getPosition();
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
  return BezierSurfaceMesh::createC2(controlPointsPositions, _patches.sCount,
                                     _patches.tCount);
}

BezierSurfaceC2::BezierSurfaceC2(const std::vector<algebra::Vec3f> &positions) {
  _name = "BezierCurveC2_" + std::to_string(_id++);

  for (const auto &controlPoint : positions) {
    auto point = std::make_unique<PointEntity>(controlPoint);
    point->surfacePoint() = true;
    subscribe(*point);
    _points.emplace_back(std::move(point));
  }
}

std::shared_ptr<BezierSurfaceC2>
BezierSurfaceC2::createFlat(const algebra::Vec3f &position, uint32_t u_patches,
                            uint32_t v_patches) {

  std::vector<algebra::Vec3f> controlPoints;

  const float length = 0.3f;
  const uint32_t u_points = 3 + u_patches;
  const uint32_t v_points = 3 + v_patches;

  controlPoints.reserve(u_points * v_points);
  for (uint32_t i = 0; i < u_points; ++i) {
    for (uint32_t j = 0; j < v_points; ++j) {
      controlPoints.emplace_back(position[0] + float(i) / 3.f * length,
                                 position[1] + float(j) / 3.f * length,
                                 position[2]);
    }
  }
  auto flatSurface = new BezierSurfaceC2(controlPoints);
  flatSurface->_patches.sCount = u_patches;
  flatSurface->_patches.tCount = v_patches;
  flatSurface->updateBezierSurface();

  flatSurface->_mesh = flatSurface->generateMesh();

  return std::shared_ptr<BezierSurfaceC2>(flatSurface);
}

std::shared_ptr<BezierSurfaceC2>
BezierSurfaceC2::createCylinder(const algebra::Vec3f &position, float r,
                                float h) {
  const uint32_t u_patches = static_cast<uint32_t>(h / 0.5f);
  const uint32_t v_patches = 2;

  const uint32_t u_points = 3 + u_patches;
  const uint32_t v_points = 3 + v_patches;

  std::vector<algebra::Vec3f> controlPoints;
  controlPoints.reserve(u_points * v_points);

  for (uint32_t i = 0; i < u_points; ++i) {
    float u_ratio = float(i) / float(u_points - 3);
    float angle = u_ratio * 2.0f * M_PI;

    float x_circle = std::cos(angle) * r;
    float y_circle = std::sin(angle) * r;

    for (uint32_t j = 0; j < v_points; ++j) {
      float v_ratio = float(j) / float(v_points - 1);
      float z = v_ratio * h;

      controlPoints.emplace_back(x_circle + position[0], y_circle + position[1],
                                 z + position[2]);
    }
  }

  auto flatSurface = new BezierSurfaceC2(controlPoints);

  flatSurface->_patches.sCount = u_patches;
  flatSurface->_patches.tCount = v_patches;
  flatSurface->updateBezierSurface();

  flatSurface->_mesh = flatSurface->generateMesh();

  return std::shared_ptr<BezierSurfaceC2>(flatSurface);
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