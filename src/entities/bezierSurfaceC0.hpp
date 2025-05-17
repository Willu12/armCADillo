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
      _points.emplace_back(std::move(point));
    }
    _mesh = generateMesh();
  }

private:
  std::unique_ptr<BezierSurfaceMesh> generateMesh() {
    std::vector<algebra::Vec3f> controlPointsPositions(_points.size());

    for (const auto &[i, point] : _points | std::views::enumerate) {
      controlPointsPositions[i] = point->getPosition();
    }
    _mesh = BezierSurfaceMesh::create(controlPointsPositions);
  }
};