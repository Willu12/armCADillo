#pragma once
#include "IEntity.hpp"
#include "ISubscriber.hpp"
#include "imgui.h"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class BezierCurve : public IEntity, public ISubscriber {
public:
  explicit BezierCurve(
      const std::vector<std::reference_wrapper<PointEntity>> points)
      : _mesh(generateMesh()) {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    for (auto &p : points) {
      p.get().subscribe(*this);
      _points.push_back(p);
    }
  }

  void addPoint(PointEntity &point) {
    point.subscribe(*this);
    _points.push_back(point);
    if (_points.size() % 3 == 1)
      updateMesh();
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
  const Mesh &getMesh() const override { return *_mesh; }

  bool renderSettings() override {
    ImGui::InputText("Name", &getName());
    ImGui::Checkbox("Show Polygonal Line", &_showPolyLine);

    for (const auto &point : _points) {
      ImGui::Selectable(point.get().getName().c_str(), false);
    }

    return false;
  }
  bool showPolyLine() const { return _showPolyLine; }

private:
  inline static int _id;
  std::vector<std::reference_wrapper<const PointEntity>> _points;
  std::unique_ptr<Mesh> _mesh;
  bool _showPolyLine = false;

  std::unique_ptr<Mesh> generateMesh() {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i < _points.size(); ++i) {
      auto worldPosition = _points[i].get().getPosition();
      vertices.push_back(worldPosition[0]);
      vertices.push_back(worldPosition[1]);
      vertices.push_back(worldPosition[2]);
    }
    for (uint32_t i = 0; i + 3 < _points.size(); i += 3) {
      indices.push_back(i);
      indices.push_back(i + 1);
      indices.push_back(i + 2);
      indices.push_back(i + 3);
    }

    return Mesh::create(vertices, indices);
  }
};