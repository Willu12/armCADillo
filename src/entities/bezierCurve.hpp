#pragma once
#include "IEntity.hpp"
#include "IMeshable.hpp"
#include "ISubscriber.hpp"
#include "IVisitor.hpp"
#include "bezierCurveMesh.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <vector>

#include "gui.hpp"

class BezierCurve : public IEntity, public ISubscriber {
public:
  explicit BezierCurve(
      const std::vector<std::reference_wrapper<PointEntity>> points) {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    for (auto &p : points) {
      _points.push_back(p);
      subscribe(p);
    }
    _mesh = generateMesh();
  }

  void addPoint(const PointEntity &point) {
    point.subscribe(*this);
    _points.push_back(point);
    updateMesh();
  }

  void removePoint(const PointEntity &point) { onSubscribableDestroyed(point); }

  std::vector<std::reference_wrapper<const PointEntity>> getPoints() const {
    return _points;
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierCurve(*this);
  }

  void updateMesh() override { _mesh = generateMesh(); };
  void update() override { updateMesh(); }
  void onSubscribableDestroyed(const ISubscribable &publisher) override {
    const PointEntity &point = static_cast<const PointEntity &>(publisher);
    auto it =
        std::find_if(_points.begin(), _points.end(),
                     [&point](const auto &p) { return &p.get() == &point; });
    _points.erase(it);
    update();
  }
  const IMeshable &getMesh() const override { return *_mesh; }
  bool &showPolyLine() { return _showPolyLine; }

private:
  inline static int _id;
  std::vector<std::reference_wrapper<const PointEntity>> _points;
  std::unique_ptr<BezierMesh> _mesh;
  bool _showPolyLine = false;

  std::unique_ptr<BezierMesh> generateMesh() {
    std::vector<algebra::Vec3f> vertices;
    for (auto &point : _points)
      vertices.push_back(point.get().getPosition());

    return BezierMesh::create(vertices);
  }
};