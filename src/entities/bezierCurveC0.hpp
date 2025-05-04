#pragma once

#include "IVisitor.hpp"
#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"
#include <cstdio>
#include <functional>
#include <memory>
#include <vector>

#include "gui.hpp"

class BezierCurveC0 : public BezierCurve {
public:
  explicit BezierCurveC0(
      const std::vector<std::reference_wrapper<PointEntity>> points) {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    for (const auto &p : points) {
      _points.emplace_back(p);
      subscribe(p);
    }
    _mesh = generateMesh();
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierCurve(*this);
  }

private:
  inline static int _id;

  std::unique_ptr<BezierMesh> generateMesh() override {
    std::vector<algebra::Vec3f> vertices;
    vertices.reserve(_points.size());
    for (const auto &point : _points)
      vertices.push_back(point.get().getPosition());

    return BezierMesh::create(vertices);
  }
};