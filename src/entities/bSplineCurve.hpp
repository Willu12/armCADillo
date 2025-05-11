#pragma once

#include "bezierCurve.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include "virtualPoint.hpp"
#include <memory>
#include <unistd.h>
#include <vector>

class BSplineCurve : public BezierCurve {
public:
  explicit BSplineCurve(
      const std::vector<std::reference_wrapper<PointEntity>> &points) {
    _name = "BSplineCurve_" + std::to_string(_id++);
    for (const auto &p : points) {
      _points.emplace_back(p);
      subscribe(p);
    }
    _bezierPoints = bezierPoints();
    _mesh = generateMesh();
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBSplineCurve(*this);
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
    addBezierPoints();
    //_bezierPoints = bezierPoints();
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
    if (_bezierPoints.size() < 4 || _points.size() < 4)
      return;

    algebra::Vec3f delta = _bezierPoints[index]->getPosition() - pos;

    int segment = std::max<int>(static_cast<int>(index) - 1, 0) / 3;

    if (segment + 3 >= _points.size())
      return;

    auto &D0 = _points[segment].get();
    auto &D1 = _points[segment + 1].get();
    auto &D2 = _points[segment + 2].get();
    auto &D3 = _points[segment + 3].get();

    int mod = static_cast<int>(index) % 3;

    if (mod == 0) {
      if (index == 0) {
        D0.setPositionWithoutNotify(D0.getPosition() + delta * 3.0f);
      } else {
        D3.setPositionWithoutNotify(D3.getPosition() + delta * 3.0f);
      }
    } else if (mod == 1) {
      D1.setPositionWithoutNotify(D1.getPosition() + delta * 2.0f);
      D2.setPositionWithoutNotify(D2.getPosition() + delta * 1.0f);
    } else if (mod == 2) {
      D1.setPositionWithoutNotify(D1.getPosition() + delta * 1.0f);
      D2.setPositionWithoutNotify(D2.getPosition() + delta * 2.0f);
    }
    recalculateBezierPoints();
    updateMesh();
  }

private:
  inline static int _id;
  std::vector<std::shared_ptr<VirtualPoint>> _bezierPoints;
  bool _showBezierPoints = false;

  void recalculateBezierPoints() {
    if (_points.size() < 4)
      return;

    const long currentBezierCount = 3 * _points.size() - 8;
    const auto bezierRemoveCount = _bezierPoints.size() - currentBezierCount;
    _bezierPoints.erase(_bezierPoints.end() - bezierRemoveCount,
                        _bezierPoints.end());

    algebra::Vec3f g = _points[0].get().getPosition() * (1.f / 3.f) +
                       _points[1].get().getPosition() * (2.f / 3.f);
    algebra::Vec3f f;
    algebra::Vec3f e;

    std::size_t bezierIndex = 0;

    for (std::size_t i = 0; i < _points.size() - 3; ++i) {
      auto p0 = _points[i].get().getPosition();
      auto p1 = _points[i + 1].get().getPosition();
      auto p2 = _points[i + 2].get().getPosition();
      auto p3 = _points[i + 3].get().getPosition();
      if (i == 0) {
        _bezierPoints[bezierIndex++]->updatePositionNoNotify(
            (p0 + p1 * 4 + p2) / 6.f);
      }
      _bezierPoints[bezierIndex++]->updatePositionNoNotify((p1 * 2 + p2 * 1) /
                                                           3.f);
      _bezierPoints[bezierIndex++]->updatePositionNoNotify((p1 + p2 * 2) / 3.f);
      _bezierPoints[bezierIndex++]->updatePositionNoNotify((p1 + p2 * 4 + p3) /
                                                           6.f);
    }
  }

  std::vector<std::shared_ptr<VirtualPoint>> bezierPoints() {
    std::vector<std::shared_ptr<VirtualPoint>> bezierPoints;

    if (_points.size() < 4)
      return bezierPoints;

    for (int i = 0; i < _points.size() * 3 - 8; ++i) {
      bezierPoints.emplace_back(
          std::make_shared<VirtualPoint>(algebra::Vec3f()));
    }

    for (const auto &p : bezierPoints) {
      p->subscribe(*this);
    }
    _bezierPoints = bezierPoints;
    recalculateBezierPoints();
    return _bezierPoints;
  }

  void addBezierPoints() {
    for (int i = 0; i < 3; ++i) {
      auto bernsteinPoint = std::make_shared<VirtualPoint>(algebra::Vec3f());
      bernsteinPoint->subscribe(*this);
      _bezierPoints.emplace_back(bernsteinPoint);
    }
    recalculateBezierPoints();
  }

  std::unique_ptr<BezierMesh> generateMesh() override {
    std::vector<algebra::Vec3f> bezierPositions;
    bezierPositions.reserve(_bezierPoints.size());
    for (const auto &p : _bezierPoints)
      bezierPositions.emplace_back(p->getPosition());

    return BezierMesh::create(bezierPositions);
  }
};