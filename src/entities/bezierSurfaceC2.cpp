#include "bezierSurfaceC2.hpp"
#include "vec.hpp"
#include <array>

void BezierSurfaceC2::updateBezierSurface() {
  _bezierControlPoints.clear();

  const uint32_t uPatches = _patches.sCount;
  const uint32_t vPatches = _patches.tCount;
  const uint32_t uPoints = 3 * _patches.sCount + 1;
  const uint32_t vPoints = 3 * _patches.tCount + 1;

  std::vector<algebra::Vec3f> fullDeBoorPoints = getRepeatingDeBoorPoints();

  std::vector<std::vector<algebra::Vec3f>> deboorMatrix(
      uPoints, std::vector<algebra::Vec3f>(vPoints));
  for (uint32_t i = 0; i < uPoints; ++i) {
    for (uint32_t j = 0; j < vPoints; ++j) {
      deboorMatrix[i][j] = _points[i * vPoints + j]->getPosition();
    }
  }

  // Step 2: Prepare empty Bézier control point matrix
  std::vector<std::vector<algebra::Vec3f>> bezierMatrix(
      uPoints, std::vector<algebra::Vec3f>(vPoints));

  // Step 3: Slide a 4×4 window and convert each patch independently
  for (uint32_t uPatch = 0; uPatch < uPatches; ++uPatch) {
    for (uint32_t vPatch = 0; vPatch < vPatches; ++vPatch) {

      // Collect 4x4 patch
      algebra::Vec3f patch[4][4];
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          patch[i][j] = deboorMatrix[uPatch * 3 + i][vPatch * 3 + j];
        }
      }

      // Convert each row from De Boor to Bézier
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

      // Convert each column from De Boor to Bézier
      algebra::Vec3f finalPatch[4][4];
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

      // Write patch into Bézier matrix at the correct location
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          const uint32_t u = uPatch * 3 + i;
          const uint32_t v = vPatch * 3 + j;
          bezierMatrix[u][v] = finalPatch[i][j];
        }
      }
    }
  }

  // Step 4: Flatten bezierMatrix in column-major order
  for (uint32_t i = 0; i < uPoints; ++i) {
    for (uint32_t j = 0; j < vPoints; ++j) {
      _bezierControlPoints.push_back(bezierMatrix[i][j]);
    }
  }
}

std::array<algebra::Vec3f, 16> BezierSurfaceC2::convertDeBoorPatchToBernstein(
    const std::array<algebra::Vec3f, 16> &deBoorPatch) {
  algebra::Vec3f temp[4][4];

  // Convert rows
  for (int i = 0; i < 4; ++i) {
    const algebra::Vec3f &d0 = deBoorPatch[i * 4 + 0];
    const algebra::Vec3f &d1 = deBoorPatch[i * 4 + 1];
    const algebra::Vec3f &d2 = deBoorPatch[i * 4 + 2];
    const algebra::Vec3f &d3 = deBoorPatch[i * 4 + 3];

    temp[i][0] = (d0 + d1 * 4.f + d2) / 6.f;
    temp[i][1] = (4.f * d1 + 2.f * d2) / 6.f;
    temp[i][2] = (2.f * d1 + 4.f * d2) / 6.f;
    temp[i][3] = (d1 + 4.f * d2 + d3) / 6.f;
  }
  std::array<algebra::Vec3f, 16> bezier;
  // Convert columns of temp
  for (int j = 0; j < 4; ++j) {
    const algebra::Vec3f &t0 = temp[0][j];
    const algebra::Vec3f &t1 = temp[1][j];
    const algebra::Vec3f &t2 = temp[2][j];
    const algebra::Vec3f &t3 = temp[3][j];

    bezier[0 * 4 + j] = (t0 + 4.f * t1 + t2) / 6.f;
    bezier[1 * 4 + j] = (4.f * t1 + 2.f * t2) / 6.f;
    bezier[2 * 4 + j] = (2.f * t1 + 4.f * t2) / 6.f;
    bezier[3 * 4 + j] = (t1 + 4.f * t2 + t3) / 6.f;
  }
  return bezier;
}

std::unique_ptr<BezierSurfaceMesh> BezierSurfaceC2::generateMesh() {
  std::vector<float> controlPointsPositions(_bezierControlPoints.size() * 3);

  for (const auto &[i, point] : _bezierControlPoints | std::views::enumerate) {
    controlPointsPositions[3 * i] = point[0];
    controlPointsPositions[3 * i + 1] = point[1];
    controlPointsPositions[3 * i + 2] = point[2];
  }
  return BezierSurfaceMesh::create(controlPointsPositions, _patches.sCount,
                                   _patches.tCount);
}

BezierSurfaceC2::BezierSurfaceC2(const std::vector<algebra::Vec3f> &positions) {
  _name = "BezierCurveC0_" + std::to_string(_id++);

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

  auto flatSurface = new BezierSurfaceC2(controlPoints);
  flatSurface->_patches.sCount = u_patches;
  flatSurface->_patches.tCount = v_patches;
  flatSurface->updateBezierSurface();

  flatSurface->_mesh = flatSurface->generateMesh();

  return std::shared_ptr<BezierSurfaceC2>(flatSurface);
}
/*
void BezierSurfaceC2::updateBezierSurface() {
  _bezierControlPoints.clear();

  const uint32_t uPatches = _patches.sCount;
  const uint32_t vPatches = _patches.tCount;
  const uint32_t uPoints = 3 * uPatches + 1;
  const uint32_t vPoints = 3 * vPatches + 1;

  // Stwórz macierz DeBoor z _points
  std::vector<std::vector<algebra::Vec3f>> deboorMatrix(
      uPoints, std::vector<algebra::Vec3f>(vPoints));

  for (uint32_t i = 0; i < uPoints; ++i) {
    for (uint32_t j = 0; j < vPoints; ++j) {
      deboorMatrix[i][j] = _points[i * vPoints + j]->getPosition();
    }
  }

  // Przetwarzaj każdy patch niezależnie
  for (uint32_t uPatch = 0; uPatch < uPatches; ++uPatch) {
    for (uint32_t vPatch = 0; vPatch < vPatches; ++vPatch) {

      // Wyciągnij 4×4 patch
      std::array<algebra::Vec3f, 16> deBoorPatch;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
          deBoorPatch[i * 4 + j] = deboorMatrix[uPatch * 3 + i][vPatch * 3 + j];
        }
      }

      // Konwertuj patch do Béziera
      std::array<algebra::Vec3f, 16> bezierPatch =
          convertDeBoorPatchToBernstein(deBoorPatch);

      // Zapisz wszystkie 16 punktów tego patcha
      _bezierControlPoints.insert(_bezierControlPoints.end(),
                                  bezierPatch.begin(), bezierPatch.end());
    }
  }
}
*/

std::vector<algebra::Vec3f> BezierSurfaceC2::getRepeatingDeBoorPoints() {
  std::vector<algebra::Vec3f> reapetingDeBoorPoints;

  const uint32_t u_points = 3 * _patches.sCount + 1;
  const uint32_t v_points = 3 * _patches.tCount + 1;

  for (uint32_t u_idx = 0; u_idx < _patches.sCount; ++u_idx) {
    for (uint32_t v_idx = 0; v_idx < _patches.tCount; ++v_idx) {
      for (uint32_t i = 0; i < 4; ++i) {
        for (uint32_t j = 0; j < 4; ++j) {
          uint32_t u = u_idx * 3 + i;
          uint32_t v = v_idx * 3 + j;
          uint32_t idx = u * v_points + v;

          reapetingDeBoorPoints.emplace_back(_points[idx]->getPosition());
        }
      }
    }
  }

  return reapetingDeBoorPoints;
}