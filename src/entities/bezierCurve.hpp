#pragma once

#include <algorithm>
#include <stdexcept>

#include "IEntity.hpp"
#include "ISubscriber.hpp"
#include "bezierCurveMesh.hpp"
#include "pointEntity.hpp"

class BezierCurve : public IEntity, public ISubscriber {
public:
  void addPoint(const PointEntity &point) {
    subscribe(point);
    _points.emplace_back(point);
    updateMesh();
  }
  void removePoint(const PointEntity &point) { onSubscribableDestroyed(point); }
  std::vector<std::reference_wrapper<const PointEntity>> getPoints() const {
    return _points;
  }
  void updateMesh() override { _mesh = generateMesh(); };
  void update() override { updateMesh(); }
  void onSubscribableDestroyed(const ISubscribable &publisher) override {

    const auto *point = dynamic_cast<const PointEntity *>(&publisher);
    if (!point) {
      throw std::runtime_error("Unexpected publisher type: " +
                               std::string(typeid(publisher).name()));
    }
    auto it = std::ranges::find_if(
        _points, [&point](const auto &p) { return &p.get() == point; });
    _points.erase(it);

    auto pub_it = std::ranges::find_if(
        _publishers, [&point](const auto &p) { return &p.get() == point; });
    pub_it->get().removeSubscriber(*this);
    _publishers.erase(pub_it);
    update();
  }
  const IMeshable &getMesh() const override { return *_mesh; }
  bool &showPolyLine() { return _showPolyLine; }

protected:
  std::vector<std::reference_wrapper<const PointEntity>> _points;
  std::unique_ptr<BezierMesh> _mesh;
  bool _showPolyLine = false;

  virtual std::unique_ptr<BezierMesh> generateMesh() = 0;
};