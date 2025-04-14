#include "GuiVisitor.hpp"
#include "IEntity.hpp"
#include "gui.hpp"
#include "imgui.h"

#include "bezierCurve.hpp"
#include "pointEntity.hpp"
#include "torusEntity.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>

bool GuiVisitor::visitTorus(TorusEntity &torus) {
  return torus.renderSettings(_gui);
}
bool GuiVisitor::visitPoint(PointEntity &point) {
  return point.renderSettings(_gui);
}
bool GuiVisitor::visitBezierCurve(BezierCurve &bezierCurve) {
  ImGui::InputText("Name", &bezierCurve.getName());
  ImGui::Checkbox("Show Polygonal Line", &bezierCurve.showPolyLine());

  auto allPoints = _gui.getPoints();
  auto points = bezierCurve.getPoints();
  auto remainingPoints = getRemainingPoints(allPoints, points);

  renderPointList(points, "Bezier curve points");
  renderPointList(remainingPoints, "remainingPoints");

  return false;
}

bool GuiVisitor::renderBasicEntitySettings(IEntity &entity) {
  ImGui::InputText("Name", &entity.getName());
  auto &_position = entity.getPosition();
  float position[3] = {_position[0], _position[1], _position[2]};
  if (ImGui::InputFloat3("Position", position)) {
    _position = algebra::Vec3f(_position[0], _position[1], _position[2]);
    return true;
  }
  return false;
};

bool GuiVisitor::isEntitySelected(const PointEntity &entity) const {
  const PointEntity *ptr = &entity;
  return std::any_of(_selectedEntities.begin(), _selectedEntities.end(),
                     [&](const auto &e) { return &e.get() == ptr; });
}

void GuiVisitor::unselectEntity(const PointEntity &entity) {
  std::erase_if(_selectedEntities,
                [&](const auto &elem) { return &elem.get() == &entity; });
}

void GuiVisitor::renderPointList(
    const std::vector<std::reference_wrapper<const PointEntity>> &entities,
    const std::string &label) {

  ImGui::LabelText("##", "%s", label.c_str());
  for (auto &entity : entities) {
    bool isSelected = isEntitySelected(entity);
    if (ImGui::Selectable((entity.get().getName() + "##").c_str(), isSelected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected)
          unselectEntity(entity);
        else
          selectEntity(entity);
      } else {
        _selectedEntities.clear();
        selectEntity(entity);
      }
    }
  }
}

std::vector<std::reference_wrapper<const PointEntity>>
GuiVisitor::getRemainingPoints(
    const std::vector<std::reference_wrapper<const PointEntity>> &allPoints,
    const std::vector<std::reference_wrapper<const PointEntity>> &currentPoints)
    const {
  std::vector<std::reference_wrapper<const PointEntity>> remainingPoints;

  for (const auto &p : allPoints) {
    if (!std::any_of(currentPoints.begin(), currentPoints.end(),
                     [&](const auto &e) { return &e.get() == &p.get(); }))
      remainingPoints.push_back(p);
  }
  return remainingPoints;
}

bool GuiVisitor::renderAddingSelectedPoints(BezierCurve &bezierCurve) {
  auto points = bezierCurve.getPoints();
  if (ImGui::Button("+ Add selected points")) {
    for (const auto &p : _selectedEntities) {
      /*
      bool alreadyIn =
          std::any_of(points.begin(), points.end(),
                      [&](const auto &q) { return &p.get() == &q.get(); });
      if (!alreadyIn) {
        bezierCurve.addPoint(
            p); // assuming bezierCurve.addPoint(ref_wrapper) exists
      }
      */
    }
  }
  return false;
}