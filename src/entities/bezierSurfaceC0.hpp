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
  BezierSurfaceC0() {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    // generate 4 points in the middle of the
    std::vector<algebra::Vec3f> controlPoints = {
        // Row 0 (v = 0)
        algebra::Vec3f(-0.5f, -0.5f, 0.0f),
        algebra::Vec3f(-0.166f, -0.5f, 0.1f),
        algebra::Vec3f(0.166f, -0.5f, 0.1f),
        algebra::Vec3f(0.5f, -0.5f, 0.0f),

        // Row 1 (v = 1/3)
        algebra::Vec3f(-0.5f, -0.166f, 0.1f),
        algebra::Vec3f(-0.166f, -0.166f, 0.2f),
        algebra::Vec3f(0.166f, -0.166f, 0.2f),
        algebra::Vec3f(0.5f, -0.166f, 0.1f),

        // Row 2 (v = 2/3)
        algebra::Vec3f(-0.5f, 0.166f, 0.1f),
        algebra::Vec3f(-0.166f, 0.166f, 0.2f),
        algebra::Vec3f(0.166f, 0.166f, 0.2f),
        algebra::Vec3f(0.5f, 0.166f, 0.1f),

        // Row 3 (v = 1)
        algebra::Vec3f(-0.5f, 0.5f, 0.0f),
        algebra::Vec3f(-0.166f, 0.5f, 0.1f),
        algebra::Vec3f(0.166f, 0.5f, 0.1f),
        algebra::Vec3f(0.5f, 0.5f, 0.0f),
    };

    for (const auto &controlPoint : controlPoints) {
      auto point = std::make_unique<PointEntity>(controlPoint);
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
    return BezierSurfaceMesh::create(controlPointsPositions);
  }
};