#pragma once

#include <algorithm>
#include <functional>
#include <stdexcept>

#include "IGroupedEntity.hpp"
#include "ISubscriber.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"

class BezierCurve : public IGroupedEntity, public ISubscriber {
public:
  virtual void addPoint(PointEntity &point) {
    subscribe(point);
    _points.emplace_back(point);
    updateMesh();
  }
  void removePoint(PointEntity &point) { onSubscribableDestroyed(point); }
  std::vector<std::reference_wrapper<PointEntity>> getPoints() const {
    return _points;
  }
  void updateMesh() override { _mesh = generateMesh(); };
  void markToUpdate() override { _dirty = true; }
  void update() override { updateMesh(); }
  void onSubscribableDestroyed(ISubscribable &publisher) override {

    const auto *point = dynamic_cast<PointEntity *>(&publisher);
    if (!point) {
      throw std::runtime_error("Unexpected publisher type: " +
                               std::string(typeid(publisher).name()));
    }

    auto it = std::ranges::find_if(_points, [&point](const auto &p) {
      return p.get().getId() == point->getId();
    });
    if (it == _points.end()) {
      update();
      return;
    }
    _points.erase(it);

    auto pub_it = std::ranges::find_if(_publishers, [&point](auto &p) {
      const auto *pub = dynamic_cast<PointEntity *>(&p.get());
      return pub->getId() == point->getId();
    });
    pub_it->get().removeSubscriber(*this);
    _publishers.erase(pub_it);
    update();
  }
  const IMeshable &getMesh() const override { return *_mesh; }
  bool &showPolyLine() { return _showPolyLine; }
  std::vector<std::reference_wrapper<const PointEntity>>
  getPointsReferences() const override {
    std::vector<std::reference_wrapper<const PointEntity>> pointsReferences;
    pointsReferences.reserve(_points.size());
    for (const auto &point : _points) {
      pointsReferences.emplace_back(point);
    }
    return pointsReferences;
  }
  std::vector<std::reference_wrapper<PointEntity>> &
  getPointsReferences() override {
    return _points;
  }

protected:
  std::vector<std::reference_wrapper<PointEntity>> _points;
  std::unique_ptr<BezierMesh> _mesh;
  bool _showPolyLine = false;

  virtual std::unique_ptr<BezierMesh> generateMesh() = 0;
};