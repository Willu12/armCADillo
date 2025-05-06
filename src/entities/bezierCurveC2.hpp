#pragma once

#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <memory>
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
    _bezierPoints = bezierPoints();
    _mesh = generateMesh();
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierCurveC2(*this);
  }

  bool &showBezierPoints() { return _showBezierPoints; }
  const std::vector<std::shared_ptr<VirtualPoint>> &getVirtualPoints() const {
    return _bezierPoints;
  }

  void update() override {
    recalculateBezierPoints();
    updateMesh();
  }

  void addPoint(PointEntity &point) override {
    subscribe(point);
    _points.emplace_back(point);
    _bezierPoints = bezierPoints();
    updateMesh();
  }

  void updateBezier(const VirtualPoint &point, const algebra::Vec3f &pos) {

    auto it = std::ranges::find_if(
        _bezierPoints,
        [&point](const std::shared_ptr<VirtualPoint> &bezierPoint) {
          return bezierPoint.get() == &point;
        });

    if (it == _bezierPoints.end())
      return;

    size_t index = std::distance(_bezierPoints.begin(), it);
    std::size_t segmentIndex = (index - 1) / 3;

    auto delta = (pos - _bezierPoints[index]->getPosition()) * 1.5;

    /*
      if (index % 3 == 0) {
        _points[index / 3 + 1].get().setPositionWithoutNotify(
            _points[index / 3 + 1].get().getPosition() - delta * 1.5f);
      } else {
        _points[segmentIndex + 1].get().setPositionWithoutNotify(
            _points[segmentIndex + 1].get().getPosition() - delta);
        _points[segmentIndex + 2].get().setPositionWithoutNotify(
            _points[segmentIndex + 2].get().getPosition() - delta);
      }
      */
    _points[(index + 1) / 3 + 1].get().setPositionWithoutNotify(
        _points[(index + 1) / 3 + 1].get().getPosition() - delta);

    updateMesh();
  }

private:
  inline static int _id;
  std::vector<std::shared_ptr<VirtualPoint>> _bezierPoints;
  bool _showBezierPoints = false;

  void recalculateBezierPoints() {
    if (_points.size() < 4)
      return;

    std::size_t bezierIndex = 0;

    for (std::size_t i = 0; i < _points.size() - 3; ++i) {
      auto p0 = _points[i].get().getPosition();
      auto p1 = _points[i + 1].get().getPosition();
      auto p2 = _points[i + 2].get().getPosition();
      auto p3 = _points[i + 3].get().getPosition();
      if (i == 0) {
        _bezierPoints[bezierIndex++]->updatePositionNoNotify(
            (p0 + p1 * 4 + p2) / 6);
      }
      _bezierPoints[bezierIndex++]->updatePositionNoNotify((p1 * 4 + p2 * 2) /
                                                           6);
      _bezierPoints[bezierIndex++]->updatePositionNoNotify((p1 * 2 + p2 * 4) /
                                                           6);
      _bezierPoints[bezierIndex++]->updatePositionNoNotify((p1 + p2 * 4 + p3) /
                                                           6);
    }
  }
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

      if (i == 0) {
        bezierPoints.push_back(
            std::make_shared<VirtualPoint>(((p0 + p1 * 4 + p2) / 6)));
      }
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
    std::vector<algebra::Vec3f> bezierPositions;
    bezierPositions.reserve(_bezierPoints.size());
    for (const auto &p : _bezierPoints)
      bezierPositions.emplace_back(p->getPosition());

    return BezierMesh::create(bezierPositions);
  }
};