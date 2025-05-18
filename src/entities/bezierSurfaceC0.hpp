#pragma once
#include "bezierSurface.hpp"
#include "bezierSurfaceMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <memory>
#include <ranges>
#include <vector>

class BezierSurfaceC0 : public BezierSurface {
public:
  explicit BezierSurfaceC0(const algebra::Vec3f &position, uint32_t u_patches,
                           uint32_t v_patches) {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    std::vector<algebra::Vec3f> controlPointsPositions;

    const float length = 1.0f;
    const uint32_t u_points = 3 * u_patches + 1;
    const uint32_t v_points = 3 * v_patches + 1;
    _patches.sCount = u_patches;
    _patches.tCount = v_patches;

    controlPointsPositions.reserve(u_points * v_points);
    for (uint32_t i = 0; i < u_points; ++i) {
      for (uint32_t j = 0; j < v_points; ++j) {
        controlPointsPositions.emplace_back(
            position[0] + float(i) / 3.f * length,
            position[1] + float(j) / 3.f * length, position[2]);
      }
    }
    /*
    for (int u_index = 0; u_index < u_patches; ++u_index) {
      for (int v_index = 0; v_index < v_patches; ++v_index) {
        for (int i = 0; i < 4; ++i) {
          for (int j = 0; j < 4; ++j) {
            if (u_index > 0)
              ++i;
            if (v_index > 0)
              ++j;

            // teraz dodajemy punkt
            controlPointsPositions.emplace_back(
                u_index * length + float(i) / 4.f * length,
                v_index * length + float(j) / 4.f * length, 0.f);
          }
        }
      }
    }
    */
    for (const auto &controlPoint : controlPointsPositions) {
      auto point = std::make_unique<PointEntity>(controlPoint);
      point->surfacePoint() = true;
      subscribe(*point);
      _points.emplace_back(std::move(point));
    }
    _mesh = generateMesh();
  }

  explicit BezierSurfaceC0(const algebra::Vec3f &position) {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    // generate 4 points in the middle of the
    std::vector<algebra::Vec3f> controlPoints = {
        // Row 0 (v = 0)
        algebra::Vec3f(-0.5f, -0.5f, 0.0f) + position,
        algebra::Vec3f(-0.166f, -0.5f, 0.1f) + position,
        algebra::Vec3f(0.166f, -0.5f, 0.1f) + position,
        algebra::Vec3f(0.5f, -0.5f, 0.0f) + position,

        // Row 1 (v = 1/3)
        algebra::Vec3f(-0.5f, -0.166f, 0.1f) + position,
        algebra::Vec3f(-0.166f, -0.166f, 0.2f) + position,
        algebra::Vec3f(0.166f, -0.166f, 0.2f) + position,
        algebra::Vec3f(0.5f, -0.166f, 0.1f) + position,

        // Row 2 (v = 2/3)
        algebra::Vec3f(-0.5f, 0.166f, 0.1f) + position,
        algebra::Vec3f(-0.166f, 0.166f, 0.2f) + position,
        algebra::Vec3f(0.166f, 0.166f, 0.2f) + position,
        algebra::Vec3f(0.5f, 0.166f, 0.1f) + position,

        // Row 3 (v = 1)
        algebra::Vec3f(-0.5f, 0.5f, 0.0f) + position,
        algebra::Vec3f(-0.166f, 0.5f, 0.1f) + position,
        algebra::Vec3f(0.166f, 0.5f, 0.1f) + position,
        algebra::Vec3f(0.5f, 0.5f, 0.0f) + position,
    };

    for (const auto &controlPoint : controlPoints) {
      auto point = std::make_unique<PointEntity>(controlPoint);
      point->surfacePoint() = true;
      subscribe(*point);
      _points.emplace_back(std::move(point));
    }
    _mesh = generateMesh();
  }
  void updateMesh() override { _mesh = generateMesh(); }

private:
  inline static int _id = 0;
  std::unique_ptr<BezierSurfaceMesh> generateMesh() {
    std::vector<float> controlPointsPositions(_points.size() * 3);

    for (const auto &[i, point] : _points | std::views::enumerate) {
      controlPointsPositions[3 * i] = point->getPosition()[0];
      controlPointsPositions[3 * i + 1] = point->getPosition()[1];
      controlPointsPositions[3 * i + 2] = point->getPosition()[2];
    }
    return BezierSurfaceMesh::create(controlPointsPositions, _patches.sCount,
                                     _patches.tCount);
  }
};