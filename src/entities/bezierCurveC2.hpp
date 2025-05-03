#pragma once

#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <unistd.h>
#include <vector>

class BezierCurveC2 : public BezierCurve {
public:
  explicit BezierCurveC2(
      const std::vector<std::reference_wrapper<PointEntity>> &points) {
    _name = "BezierCurveC2_" + std::to_string(_id++);
    for (const auto &p : points) {
      _points.emplace_back(p);
      subscribe(p);
    }
    _mesh = generateMesh();
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierCurveC2(*this);
  }

private:
  inline static int _id;
  std::vector<algebra::Vec3f> _bezierPoints;
  bool _showBezierPoints = false;

  std::vector<algebra::Vec3f> bezierPoints() {
    std::vector<algebra::Vec3f> bezierPoints;

    if (_points.size() < 4)
      return bezierPoints;

    bezierPoints.reserve((_points.size() - 3) * 4);
    for (std::size_t i = 0; i < _points.size() - 3; ++i) {
      auto p0 = _points[i].get().getPosition();
      auto p1 = _points[i + 1].get().getPosition();
      auto p2 = _points[i + 2].get().getPosition();
      auto p3 = _points[i + 3].get().getPosition();

      bezierPoints.emplace_back((p0 + p1 * 4 + p2) / 6);
      bezierPoints.emplace_back((p1 * 4 + p2 * 2) / 6);
      bezierPoints.emplace_back((p1 * 2 + p2 * 4) / 6);
      bezierPoints.emplace_back((p1 + p2 * 4 + p3) / 6);
    }
    return bezierPoints;
  }

  std::unique_ptr<BezierMesh> generateMesh() override {
    _bezierPoints = bezierPoints();
    return BezierMesh::create(_bezierPoints);
  }
};