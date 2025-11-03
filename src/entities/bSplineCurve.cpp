#include "bSplineCurve.hpp"

BSplineCurve::BSplineCurve(
    const std::vector<std::reference_wrapper<PointEntity>> &points) {
  _id = kClassId++;
  _name = "BSplineCurve_" + std::to_string(_id++);
  for (const auto &p : points) {
    _points.emplace_back(p);
    subscribe(p);
  }
  initBezierPoints();
  _mesh = generateMesh();
}

bool BSplineCurve::acceptVisitor(IVisitor &visitor) {
  return visitor.visitBSplineCurve(*this);
}

bool &BSplineCurve::showBezierPoints() { return _showBezierPoints; }

std::vector<VirtualPoint *> BSplineCurve::getVirtualPoints() const {
  std::vector<VirtualPoint *> points;
  points.reserve(_bezierPoints.size());
  for (const auto &p : _bezierPoints) {
    points.push_back(p.get());
  }
  return points;
}

void BSplineCurve::update() {
  recalculateBezierPoints();
  updateMesh();
}

void BSplineCurve::addPoint(PointEntity &point) {
  subscribe(point);
  _points.emplace_back(point);
  addBezierPoints();
  //_bezierPoints = bezier_points();
  updateMesh();
}

void BSplineCurve::updateBezier(const VirtualPoint &point,
                                const algebra::Vec3f &pos) {
  auto it = std::ranges::find_if(
      _bezierPoints,
      [&point](const std::unique_ptr<VirtualPoint> &bezierPoint) {
        return bezierPoint->getId() == point.getId();
      });

  if (it == _bezierPoints.end()) {
    return;
  }

  size_t index = std::distance(_bezierPoints.begin(), it);
  if (_bezierPoints.size() < 4 || _points.size() < 4) {
    return;
  }

  algebra::Vec3f delta = _bezierPoints[index]->getPosition() - pos;

  int segment = std::max<int>(static_cast<int>(index) - 1, 0) / 3;

  if (segment + 3 >= _points.size()) {
    return;
  }

  auto &d_0 = _points[segment].get();
  auto &d_1 = _points[segment + 1].get();
  auto &d_2 = _points[segment + 2].get();
  auto &d_3 = _points[segment + 3].get();

  int mod = static_cast<int>(index) % 3;

  if (mod == 0) {
    if (index == 0) {
      d_0.setPositionWithoutNotify(d_0.getPosition() + delta * 3.0f);
    } else {
      d_3.setPositionWithoutNotify(d_3.getPosition() + delta * 3.0f);
    }
  } else if (mod == 1) {
    d_1.setPositionWithoutNotify(d_1.getPosition() + delta * 2.0f);
    d_2.setPositionWithoutNotify(d_2.getPosition() + delta * 1.0f);
  } else if (mod == 2) {
    d_1.setPositionWithoutNotify(d_1.getPosition() + delta * 1.0f);
    d_2.setPositionWithoutNotify(d_2.getPosition() + delta * 2.0f);
  }
  recalculateBezierPoints();
  updateMesh();
}

void BSplineCurve::recalculateBezierPoints() {
  if (_points.size() < 4) {
    return;
  }

  const auto current_bezier_points_count = 3 * _points.size() - 8;
  const auto removed_bezier_points_count =
      _bezierPoints.size() - current_bezier_points_count;
  _bezierPoints.erase(_bezierPoints.end() -
                          static_cast<int64_t>(removed_bezier_points_count),
                      _bezierPoints.end());

  std::size_t bezier_index = 0;

  for (std::size_t i = 0; i < _points.size() - 3; ++i) {
    auto p0 = _points[i].get().getPosition();
    auto p1 = _points[i + 1].get().getPosition();
    auto p2 = _points[i + 2].get().getPosition();
    auto p3 = _points[i + 3].get().getPosition();
    if (i == 0) {
      _bezierPoints[bezier_index++]->updatePositionNoNotify((p0 + p1 * 4 + p2) /
                                                            6.f);
    }
    _bezierPoints[bezier_index++]->updatePositionNoNotify((p1 * 2 + p2 * 1) /
                                                          3.f);
    _bezierPoints[bezier_index++]->updatePositionNoNotify((p1 + p2 * 2) / 3.f);
    _bezierPoints[bezier_index++]->updatePositionNoNotify((p1 + p2 * 4 + p3) /
                                                          6.f);
  }
}

std::vector<std::unique_ptr<VirtualPoint>> BSplineCurve::initBezierPoints() {
  std::vector<std::unique_ptr<VirtualPoint>> bezier_points;

  if (_points.size() < 4) {
    return bezier_points;
  }

  for (int i = 0; i < _points.size() * 3 - 8; ++i) {
    bezier_points.emplace_back(
        std::make_unique<VirtualPoint>(algebra::Vec3f()));
  }

  for (const auto &p : bezier_points) {
    p->subscribe(*this);
  }
  _bezierPoints = std::move(bezier_points);
  recalculateBezierPoints();

  return bezier_points;
}

void BSplineCurve::addBezierPoints() {
  for (int i = 0; i < 3; ++i) {
    auto bernstein_points = std::make_unique<VirtualPoint>(algebra::Vec3f());
    bernstein_points->subscribe(*this);
    _bezierPoints.emplace_back(std::move(bernstein_points));
  }

  recalculateBezierPoints();
}

std::unique_ptr<BezierMesh> BSplineCurve::generateMesh() {
  std::vector<algebra::Vec3f> bezier_points_positions;
  bezier_points_positions.reserve(_bezierPoints.size());
  for (const auto &p : _bezierPoints) {
    bezier_points_positions.emplace_back(p->getPosition());
  }

  return BezierMesh::create(bezier_points_positions);
}