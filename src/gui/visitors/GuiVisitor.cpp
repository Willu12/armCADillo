#include "GuiVisitor.hpp"
#include "gui.hpp"
#include "imgui.h"

#include "bezierCurve.hpp"
#include "pointEntity.hpp"
#include "torusEntity.hpp"

bool GuiVisitor::visitTorus(TorusEntity &torus) {
  return torus.renderSettings(_gui);
}
bool GuiVisitor::visitPoint(PointEntity &point) {
  return point.renderSettings(_gui);
}
bool GuiVisitor::visitBezierCurve(BezierCurve &bezierCurve) {
  ImGui::InputText("Name", &bezierCurve.getName());
  ImGui::Checkbox("Show Polygonal Line", &bezierCurve.showPolyLine());

  auto points = bezierCurve.getPoints();
  auto remainingPoints =
      intersection(_gui.getPoints(), bezierCurve.getPoints());

  for (int i = 0; i < points.size(); ++i) {
    bool isSelected =
        std::find_if(_selectedEntities.begin(), _selectedEntities.end(),
                     [&](const auto &elem) {
                       return &elem == &_selectedEntities[i];
                     }) != _selectedEntities.end();

    if (ImGui::Selectable((points[i].get().getName() + "##").c_str(),
                          isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected)
          unselectEntity(i, points);
        else
          selectEntity(i, points);

      } else {
        _selectedEntities.clear();
        selectEntity(i, points);
      }
    }
  }

  ImGui::LabelText("##Remaining Labels", "Remaining Points:\n");
  for (const auto &point : remainingPoints) {
    std::string label = point.get().getName() + "##";
    ImGui::Selectable(label.c_str(), false);
  }

  return false;
}

std::vector<std::reference_wrapper<const PointEntity>> GuiVisitor::intersection(
    std::vector<std::reference_wrapper<const PointEntity>> v1,
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
};

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