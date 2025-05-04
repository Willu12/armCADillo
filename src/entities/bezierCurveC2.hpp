#pragma once

#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <memory>
#include <unistd.h>
#include <utility>
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

  bool &showBezierPoints() { return _showBezierPoints; }
  const std::vector<std::shared_ptr<VirtualPoint>> &getVirtualPoints() const {
    return _bezierPoints;
  }

  void updateBezier(const VirtualPoint &point) {

    auto it = std::ranges::find_if(
        _bezierPoints,
        [&point](const std::shared_ptr<VirtualPoint> &bezierPoint) {
          return bezierPoint.get() == &point;
        });

    if (it == _bezierPoints.end())
      return;

    size_t index = std::distance(_bezierPoints.begin(), it);
    std::size_t segmentIndex = index / 4;
    std::size_t knotIndex = index % 4;
    /*
        if (knotIndex == 0) {
          // Modify the De Boor points on the left
          _points[segmentIndex].get().getPosition() =
              _points[segmentIndex].get().getPosition() + point.getPosition();
        } else if (knotIndex == 3) {
          // Modify the De Boor points on the right
          _points[segmentIndex + 1].get().getPosition() =
              _points[segmentIndex + 1].get().getPosition() +
       point.getPosition(); } else {
          // If it's one of the middle Bezier points, modify the related De Boor
          // points
          _points[segmentIndex + 1].get().getPosition() =
              _points[segmentIndex + 1].get().getPosition() +
       point.getPosition(); _points[segmentIndex + 2].get().getPosition() =
              _points[segmentIndex + 1].get().getPosition() +
       point.getPosition();
        }
        */
  }

private:
  inline static int _id;
  std::vector<std::shared_ptr<VirtualPoint>> _bezierPoints;
  bool _showBezierPoints = false;

  std::vector<std::shared_ptr<VirtualPoint>> bezierPoints() {
    std::vector<std::shared_ptr<VirtualPoint>> bezierPoints;

    if (_points.size() < 4)
      return bezierPoints;

    bezierPoints.reserve((_points.size() - 3) * 4);
    for (std::size_t i = 0; i < _points.size() - 3; ++i) {
      auto p0 = _points[i].get().getPosition();
      auto p1 = _points[i + 1].get().getPosition();
      auto p2 = _points[i + 2].get().getPosition();
      auto p3 = _points[i + 3].get().getPosition();

      bezierPoints.push_back(
          std::make_shared<VirtualPoint>(((p0 + p1 * 4 + p2) / 6)));
      bezierPoints.push_back(
          std::make_shared<VirtualPoint>(((p1 * 4 + p2 * 2) / 6)));
      bezierPoints.push_back(
          std::make_shared<VirtualPoint>(((p1 * 2 + p2 * 4) / 6)));
      bezierPoints.push_back(
          std::make_shared<VirtualPoint>(((p1 + p2 * 4 + p3) / 6)));
    }

    for (const auto &p : bezierPoints) {
      p->subscribe(*this);
    }
    return bezierPoints;
  }

  std::unique_ptr<BezierMesh> generateMesh() override {
    _bezierPoints = bezierPoints();
    std::vector<algebra::Vec3f> bezierPositions;
    bezierPositions.reserve(_bezierPoints.size());
    for (const auto &p : _bezierPoints)
      bezierPositions.emplace_back(p->getPosition());

    return BezierMesh::create(bezierPositions);
  }
};