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

#include "gui.hpp"

class BezierCurve : public IEntity, public ISubscriber {
public:
  explicit BezierCurve(
      const std::vector<std::reference_wrapper<PointEntity>> points)
      : _mesh(generateMesh()) {
    _name = "BezierCurveC0_" + std::to_string(_id++);
    for (auto &p : points) {
      _points.push_back(p);
      subscribe(p);
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

  bool renderSettings(const GUI &gui) override {
    ImGui::InputText("Name", &getName());
    ImGui::Checkbox("Show Polygonal Line", &_showPolyLine);

    auto remainingPoints = intersection(gui.getPoints(), _points);

    for (const auto &point : _points) {
      std::string label = point.get().getName() + "##";
      ImGui::Selectable(label.c_str(), false);
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

  std::vector<std::reference_wrapper<const PointEntity>>
  intersection(std::vector<std::reference_wrapper<const PointEntity>> v1,
               std::vector<std::reference_wrapper<const PointEntity>> v2) {

    std::vector<std::reference_wrapper<const PointEntity>> v3;

    auto ptrLess = [](const std::reference_wrapper<const PointEntity> &a,
                      const std::reference_wrapper<const PointEntity> &b) {
      return &a.get() < &b.get();
    };

    std::sort(v1.begin(), v1.end(), ptrLess);
    std::sort(v2.begin(), v2.end(), ptrLess);

    std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                          std::back_inserter(v3), ptrLess);

    return v3;
  }
};