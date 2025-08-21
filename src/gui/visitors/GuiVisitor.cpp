#include "GuiVisitor.hpp"
#include "IEntity.hpp"
#include "bezierSurfaceC0.hpp"
#include "entitiesTypes.hpp"
#include "gui.hpp"
#include "imgui.h"

#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurface.hpp"
#include "interpolatingSplineC2.hpp"
#include "intersectionCurve.hpp"
#include "pointEntity.hpp"
#include "polyline.hpp"
#include "scene.hpp"
#include "torusEntity.hpp"
#include "virtualPoint.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>

bool GuiVisitor::visitTorus(TorusEntity &torus) {
  if (torus.hasIntersectionTexture())
    ImGui::Checkbox("Trim", &torus.isTrimmed());
  return torus.renderSettings(_gui);
}
bool GuiVisitor::visitPoint(PointEntity &point) {
  return point.renderSettings(_gui);
}

bool GuiVisitor::visitVirtualPoint(VirtualPoint & /*point*/) { return false; }

bool GuiVisitor::visitBezierCurve(BezierCurveC0 &bezierCurve) {
  ImGui::InputText("Name", &bezierCurve.getName());
  ImGui::Checkbox("Show Polygonal Line", &bezierCurve.showPolyLine());

  auto allPoints = _gui.getPoints();
  auto points = bezierCurve.getPoints();
  auto remainingPoints = getRemainingPoints(allPoints, points);

  renderPointList(points, "Bezier curve points");
  renderPointList(remainingPoints, "remainingPoints");
  renderAddingSelectedPoints(bezierCurve);
  ImGui::SameLine();
  renderRemovingSelectedPoints(bezierCurve);

  return false;
}

bool GuiVisitor::visitBSplineCurve(BSplineCurve &bezierCurve) {
  ImGui::InputText("Name", &bezierCurve.getName());
  ImGui::Checkbox("Show Bezier Points", &bezierCurve.showBezierPoints());
  ImGui::Checkbox("Show Polygonal Line", &bezierCurve.showPolyLine());

  auto allPoints = _gui.getPoints();
  auto points = bezierCurve.getPoints();
  auto remainingPoints = getRemainingPoints(allPoints, points);

  renderPointList(points, "Bezier curve points");
  renderPointList(remainingPoints, "remainingPoints");
  renderAddingSelectedPoints(bezierCurve);
  ImGui::SameLine();
  renderRemovingSelectedPoints(bezierCurve);

  if (bezierCurve.showBezierPoints()) {
    const auto &vPoints = bezierCurve.getVirtualPoints();
    renderVirtualPointList(vPoints);
    _gui.setVirtualPoints(vPoints, _selectedVirtualPoints);
  } else {
    _gui.clearVirtualPoints();
  }

  return false;
}

bool GuiVisitor::visitInterpolatingSplineCurve(
    InterpolatingSplineC2 &interpolatingSpline) {
  return renderCurveGui(interpolatingSpline);
}

bool GuiVisitor::visitBezierSurface(BezierSurface &bezierSurface) {
  ImGui::InputText("Name", &bezierSurface.getName());
  bool change = false;
  change |= ImGui::SliderInt("Horizontal Density",
                             &bezierSurface.getMeshDensity().s, 3, 64);
  change |= ImGui::SliderInt("Vertical Density",
                             &bezierSurface.getMeshDensity().t, 3, 64);
  ImGui::Checkbox("Show Polygonal Line", &bezierSurface.wireframe());
  if (bezierSurface.hasIntersectionTexture()) {
    ImGui::Checkbox("Trim", &bezierSurface.isTrimmed());
  }
  if (ImGui::Button("Select Elements points")) {
    for (const auto &point : bezierSurface.getPoints()) {
      _gui.selectEntity(point);
    }
  }
  return change;
}

bool GuiVisitor::visitBezierSurfaceC0(BezierSurfaceC0 &bezierSurface) {
  if (auto *p = dynamic_cast<BezierSurface *>(&bezierSurface)) {
    return visitBezierSurface(*p);
  }
  return false;
}
bool GuiVisitor::visitBezierSurfaceC2(BezierSurfaceC2 &bezierSurface) {
  if (auto *p = dynamic_cast<BezierSurface *>(&bezierSurface)) {
    visitBezierSurface(*p);
  }
  return false;
}

bool GuiVisitor::visitGregorySurface(GregorySurface &gregorySurface) {
  ImGui::InputText("Name", &gregorySurface.getName());
  ImGui::Checkbox("Show Tangent Vectors", &gregorySurface.showTangentVectors());

  auto &meshDensities = gregorySurface.getMeshDensities();
  bool change = false;
  for (int i = 0; i < meshDensities.size(); ++i) {
    ImGui::Text("Density of Quad %d", i);
    std::string labelH = "Horizontal Density##" + std::to_string(i);
    std::string labelV = "Vertical Density##" + std::to_string(i);
    change |= ImGui::SliderInt(labelH.c_str(), &meshDensities[i].s, 3, 64);
    change |= ImGui::SliderInt(labelV.c_str(), &meshDensities[i].t, 3, 64);
  }
  return change;
}

bool GuiVisitor::visitIntersectionCurve(IntersectionCurve &intersectionCurve) {
  ImGui::InputText("Name", &intersectionCurve.getName());

  auto &texture1 = intersectionCurve.getFirstTexture();
  auto &texture2 = intersectionCurve.getSecondTexture();

  static bool showFirstTexture = false;
  static bool showSecondTexture = false;
  ImGui::Checkbox("Show First intersection Texture", &showFirstTexture);
  ImGui::Checkbox(" Show Second intersection Texture", &showSecondTexture);

  if (showFirstTexture) {
    ImGui::SetNextWindowSize(ImVec2(330, 330), ImGuiCond_Always);
    ImGui::Begin("first Texture", nullptr, ImGuiWindowFlags_NoResize);

    ImVec2 imageSize(300, 300);
    ImVec2 imagePos = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)texture1.getTextureId(), imageSize);

    if (ImGui::IsItemHovered()) {
      ImVec2 mousePos = ImGui::GetMousePos();
      ImVec2 localPos =
          ImVec2(mousePos.x - imagePos.x, mousePos.y - imagePos.y);

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        texture1.floodFill(localPos.x, localPos.y, true);
      else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        texture1.floodFill(localPos.x, localPos.y, false);
    }

    ImGui::End();
  }
  if (showSecondTexture) {
    ImGui::SetNextWindowSize(ImVec2(330, 330), ImGuiCond_Always);
    ImGui::Begin("second Texture", nullptr, ImGuiWindowFlags_NoResize);

    ImVec2 imageSize(300, 300);
    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)(intptr_t)texture2.getTextureId(), imageSize);

    if (ImGui::IsItemHovered()) {
      ImVec2 mousePos = ImGui::GetMousePos();
      ImVec2 localPos =
          ImVec2(mousePos.x - imagePos.x, mousePos.y - imagePos.y);

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        texture2.floodFill(localPos.x, localPos.y, true);
      else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        texture2.floodFill(localPos.x, localPos.y, false);
    }

    ImGui::End();
  }

  if (ImGui::Button("Convert to interoplating spline")) {

    std::vector<std::reference_wrapper<PointEntity>> points;
    auto &scene = _gui.getScene();
    for (const auto &p :
         intersectionCurve.getPolyline().getSparsePoints(0.5f)) {
      auto point = std::make_shared<PointEntity>(p);
      scene.addEntity(EntityType::Point, point);
      points.emplace_back(*point);
    }
    if (intersectionCurve.isLooped() &&
        points.front().get().getId() != points.back().get().getId()) {
      points.push_back(points.front());
    }

    auto interpolatingSpline = std::make_shared<InterpolatingSplineC2>(points);
    scene.addEntity(EntityType::InterpolatingSplineCurve, interpolatingSpline);
    intersectionCurve.isDead() = true;
  }
  return false;
}

bool GuiVisitor::renderBasicEntitySettings(IEntity &entity) {
  ImGui::InputText("Name", &entity.getName());
  const auto &position = entity.getPosition();
  float guiPosition[3] = {position[0], position[1], position[2]};
  if (ImGui::InputFloat3("Position", guiPosition)) {
    entity.updatePosition(
        algebra::Vec3f(position[0], position[1], position[2]));
    return true;
  }
  return false;
};

bool GuiVisitor::isEntitySelected(const PointEntity &entity) const {
  const PointEntity *ptr = &entity;
  return std::ranges::any_of(_selectedEntities,
                             [&](const auto &e) { return &e.get() == ptr; });
}

void GuiVisitor::unselectEntity(const PointEntity &entity) {
  std::erase_if(_selectedEntities,
                [&](const auto &elem) { return &elem.get() == &entity; });
}

void GuiVisitor::selectEntity(PointEntity &entity) {
  _selectedEntities.emplace_back(entity);
}

void GuiVisitor::renderPointList(
    const std::vector<std::reference_wrapper<PointEntity>> &entities,
    const std::string &label) {

  std::unordered_map<uint32_t, uint32_t> pointCount;
  ImGui::LabelText("##", "%s", label.c_str());
  for (const auto &entity : entities) {
    bool isSelected = isEntitySelected(entity);
    auto id = entity.get().getId();
    pointCount[id]++;

    if (ImGui::Selectable(
            (entity.get().getName() + "##" + std::to_string(pointCount[id]))
                .c_str(),
            isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
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

std::vector<std::reference_wrapper<PointEntity>> GuiVisitor::getRemainingPoints(
    const std::vector<std::reference_wrapper<PointEntity>> &allPoints,
    const std::vector<std::reference_wrapper<PointEntity>> &currentPoints)
    const {
  std::vector<std::reference_wrapper<PointEntity>> remainingPoints;

  for (const auto &p : allPoints) {
    if (!std::ranges::any_of(currentPoints,
                             [&](auto &e) { return &e.get() == &p.get(); }))
      remainingPoints.push_back(p);
  }
  return remainingPoints;
}

bool GuiVisitor::renderAddingSelectedPoints(BezierCurve &bezierCurve) {
  auto points = bezierCurve.getPoints();
  if (ImGui::Button("+")) {
    for (const auto &p : _selectedEntities) {
      bool alreadyIn = std::ranges::any_of(
          points, [&](const auto &q) { return &p.get() == &q.get(); });
      if (!alreadyIn) {
        bezierCurve.addPoint(p.get());
      }
    }
  }
  return false;
}

bool GuiVisitor::renderRemovingSelectedPoints(BezierCurve &bezierCurve) {
  auto points = bezierCurve.getPoints();
  if (ImGui::Button("-")) {
    for (auto &p : _selectedEntities) {
      bool isIn = std::ranges::any_of(
          points, [&](const auto &q) { return &p.get() == &q.get(); });
      if (isIn) {
        bezierCurve.removePoint(p.get());
      }
    }
  }
  return false;
}

bool GuiVisitor::isVirtualPointSelected(const VirtualPoint &point) const {
  const VirtualPoint *ptr = &point;
  return std::ranges::any_of(_selectedVirtualPoints,
                             [&](const auto &e) { return &e.get() == ptr; });
}
void GuiVisitor::selectVirtualPoint(const VirtualPoint &point) {
  _selectedVirtualPoints.emplace_back(point);
}
void GuiVisitor::unselectVirtualPoint(const VirtualPoint &point) {
  std::erase_if(_selectedVirtualPoints,
                [&](const auto &elem) { return &elem.get() == &point; });
}

void GuiVisitor::renderVirtualPointList(
    const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints) {
  ImGui::LabelText("##", "%s", "Virtual Points");
  for (const auto &vPoint : virtualPoints) {
    bool isSelected = isVirtualPointSelected(*vPoint);
    if (ImGui::Selectable((vPoint->getName() + "##").c_str(), isSelected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected)
          unselectVirtualPoint(*vPoint);
        else
          selectVirtualPoint(*vPoint);
      } else {
        _selectedVirtualPoints.clear();
        selectVirtualPoint(*vPoint);
      }
    }
  }
}
bool GuiVisitor::renderCurveGui(BezierCurve &curve) {
  ImGui::InputText("Name", &curve.getName());
  auto allPoints = _gui.getPoints();
  auto points = curve.getPoints();
  auto remainingPoints = getRemainingPoints(allPoints, points);

  renderPointList(points, "Bezier curve points");
  renderPointList(remainingPoints, "remainingPoints");
  renderAddingSelectedPoints(curve);
  ImGui::SameLine();
  renderRemovingSelectedPoints(curve);
  return false;
}
