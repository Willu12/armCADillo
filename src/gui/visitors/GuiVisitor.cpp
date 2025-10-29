#include "GuiVisitor.hpp"
#include "IEntity.hpp"
#include "bezierSurfaceC0.hpp"
#include "entitiesTypes.hpp"
#include "gui.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"

#include "bSplineCurve.hpp"
#include "bezierCurveC0.hpp"
#include "bezierSurface.hpp"
#include "interpolatingSplineC2.hpp"
#include "intersectionCurve.hpp"
#include "intersectionTexture.hpp"
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
#include <unordered_set>
#include <vector>

bool GuiVisitor::visitTorus(TorusEntity &torus) {
  if (torus.hasIntersectionTexture()) {
    ImGui::Checkbox("Trim", &torus.isTrimmed());
  }
  return torus.renderSettings(_gui);
}
bool GuiVisitor::visitPoint(PointEntity &point) {
  return point.renderSettings(_gui);
}

bool GuiVisitor::visitVirtualPoint(VirtualPoint & /*point*/) { return false; }

bool GuiVisitor::visitBezierCurve(BezierCurveC0 &bezierCurve) {
  ImGui::InputText("Name", &bezierCurve.getName());
  ImGui::Checkbox("Show Polygonal Line", &bezierCurve.showPolyLine());

  auto all_points = _gui.getPoints();
  auto points = bezierCurve.getPoints();
  auto remaining_points = getRemainingPoints(all_points, points);

  renderPointList(points, "Bezier curve points");
  renderPointList(remaining_points, "remainingPoints");
  renderAddingSelectedPoints(bezierCurve);
  ImGui::SameLine();
  renderRemovingSelectedPoints(bezierCurve);

  return false;
}

bool GuiVisitor::visitBSplineCurve(BSplineCurve &bezierCurve) {
  ImGui::InputText("Name", &bezierCurve.getName());
  ImGui::Checkbox("Show Bezier Points", &bezierCurve.showBezierPoints());
  ImGui::Checkbox("Show Polygonal Line", &bezierCurve.showPolyLine());

  auto all_points = _gui.getPoints();
  auto points = bezierCurve.getPoints();
  auto remaining_points = getRemainingPoints(all_points, points);

  renderPointList(points, "Bezier curve points");
  renderPointList(remaining_points, "remainingPoints");
  renderAddingSelectedPoints(bezierCurve);
  ImGui::SameLine();
  renderRemovingSelectedPoints(bezierCurve);

  if (bezierCurve.showBezierPoints()) {
    const auto &virtual_points = bezierCurve.getVirtualPoints();
    renderVirtualPointList(virtual_points);
    _gui.setVirtualPoints(virtual_points, _selectedVirtualPoints);
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
    std::unordered_set<std::reference_wrapper<PointEntity>,
                       PointEntity::RefHash, PointEntity::RefEq>
        points_set;

    for (const auto &point : bezierSurface.getPoints()) {
      points_set.insert(point);
    }

    for (const auto &p : points_set) {
      _gui.selectEntity(p);
    }
  }

  return change;
}

bool GuiVisitor::visitBezierSurfaceC0(BezierSurfaceC0 &bezierSurface) {
  if (auto *p = dynamic_cast<BezierSurface *>(&bezierSurface)) {
    visitBezierSurface(*p);
  }
  if (ImGui::Button("Clone")) {
    auto points = clonePoints(bezierSurface);
    _gui.getScene().addEntity(EntityType::BezierSurfaceC0,
                              std::make_unique<BezierSurfaceC0>(
                                  points, bezierSurface.getPatches().colCount,
                                  bezierSurface.getPatches().rowCount,
                                  bezierSurface.getConnectionType()));
  }

  return false;
}
bool GuiVisitor::visitBezierSurfaceC2(BezierSurfaceC2 &bezierSurface) {
  if (auto *p = dynamic_cast<BezierSurface *>(&bezierSurface)) {
    visitBezierSurface(*p);
  }
  if (ImGui::Button("Clone")) {
    auto points = clonePoints(bezierSurface);
    _gui.getScene().addEntity(EntityType::BezierSurfaceC2,
                              std::make_unique<BezierSurfaceC2>(
                                  points, bezierSurface.getPatches().colCount,
                                  bezierSurface.getPatches().rowCount,
                                  bezierSurface.getConnectionType()));
  }
  return false;
}

bool GuiVisitor::visitGregorySurface(GregorySurface &gregorySurface) {
  ImGui::InputText("Name", &gregorySurface.getName());
  ImGui::Checkbox("Show Tangent Vectors", &gregorySurface.showTangentVectors());

  auto &mesh_densities = gregorySurface.getMeshDensities();
  bool change = false;
  for (int i = 0; i < mesh_densities.size(); ++i) {
    ImGui::Text("Density of Quad %d", i);
    std::string label_h = "Horizontal Density##" + std::to_string(i);
    std::string label_v = "Vertical Density##" + std::to_string(i);
    change |= ImGui::SliderInt(label_h.c_str(), &mesh_densities[i].s, 3, 64);
    change |= ImGui::SliderInt(label_v.c_str(), &mesh_densities[i].t, 3, 64);
  }
  return change;
}
bool GuiVisitor::visitIntersectionCurve(IntersectionCurve &intersectionCurve) {
  ImGui::InputText("Name", &intersectionCurve.getName());

  auto &texture1 = intersectionCurve.getFirstTexture();
  auto &texture2 = intersectionCurve.getSecondTexture();

  static bool show_first_texture = false;
  static bool show_second_texture = false;

  ImGui::Checkbox("Show First intersection Texture", &show_first_texture);
  ImGui::Checkbox("Show Second intersection Texture", &show_second_texture);

  auto render_texture_window = [](const std::string &window_name,
                                  bool show_texture,
                                  IntersectionTexture &texture) {
    if (show_texture) {
      ImGui::SetNextWindowSize(ImVec2(330, 330), ImGuiCond_Always);
      ImGui::Begin(window_name.c_str(), nullptr, ImGuiWindowFlags_NoResize);

      ImVec2 image_size(300, 300);
      ImVec2 image_pos = ImGui::GetCursorScreenPos();
      ImGui::Image(static_cast<ImTextureID>(
                       static_cast<intptr_t>(texture.getTextureId())),
                   image_size);

      if (ImGui::IsItemHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 local_pos =
            ImVec2(mouse_pos.x - image_pos.x, mouse_pos.y - image_pos.y);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          texture.floodFill(local_pos.x, local_pos.y, true);
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
          texture.floodFill(local_pos.x, local_pos.y, false);
        }
      }

      ImGui::End();
    }
  };

  render_texture_window("First Texture", show_first_texture, texture1);
  render_texture_window("Second Texture", show_second_texture, texture2);

  // Convert to  spline
  if (ImGui::Button("Convert to interpolating spline")) {
    std::vector<std::reference_wrapper<PointEntity>> points;
    auto &scene = _gui.getScene();

    for (const auto &p :
         intersectionCurve.getPolyline().getSparsePoints(0.5f)) {
      auto point = std::make_unique<PointEntity>(p);
      scene.addEntity(EntityType::Point, std::move(point));
      points.emplace_back(*point);
    }

    if (intersectionCurve.isLooped() &&
        points.front().get().getId() != points.back().get().getId()) {
      points.push_back(points.front());
    }

    auto interpolating_spline = std::make_unique<InterpolatingSplineC2>(points);
    scene.addEntity(EntityType::InterpolatingSplineCurve,
                    std::move(interpolating_spline));

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

  const ImVec2 list_size = ImVec2(0, 100);
  ImGui::BeginChild(label.c_str(), list_size, true,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar);

  for (const auto &entity : entities) {
    bool isSelected = isEntitySelected(entity);
    auto id = entity.get().getId();
    pointCount[id]++;

    if (ImGui::Selectable(
            (entity.get().getName() + "##" + std::to_string(pointCount[id]))
                .c_str(),
            isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected) {
          unselectEntity(entity);
        } else {
          selectEntity(entity);
        }
      } else {
        _selectedEntities.clear();
        selectEntity(entity);
      }
    }
  }

  ImGui::EndChild();
}

std::vector<std::reference_wrapper<PointEntity>> GuiVisitor::getRemainingPoints(
    const std::vector<std::reference_wrapper<PointEntity>> &allPoints,
    const std::vector<std::reference_wrapper<PointEntity>> &currentPoints)
    const {
  std::vector<std::reference_wrapper<PointEntity>> remainingPoints;

  for (const auto &p : allPoints) {
    if (!std::ranges::any_of(currentPoints,
                             [&](auto &e) { return &e.get() == &p.get(); })) {
      remainingPoints.push_back(p);
    }
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
    const std::vector<VirtualPoint *> &virtualPoints) {
  ImGui::LabelText("##", "%s", "Virtual Points");
  for (const auto &vPoint : virtualPoints) {
    bool isSelected = isVirtualPointSelected(*vPoint);
    if (ImGui::Selectable((vPoint->getName() + "##").c_str(), isSelected,
                          ImGuiSelectableFlags_AllowDoubleClick)) {
      if (ImGui::GetIO().KeyCtrl) {
        if (isSelected) {
          unselectVirtualPoint(*vPoint);
        } else {
          selectVirtualPoint(*vPoint);
        }
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

std::vector<std::reference_wrapper<PointEntity>>
GuiVisitor::clonePoints(const BezierSurface &bezierSurface) {
  std::vector<std::reference_wrapper<PointEntity>> points;
  points.reserve(bezierSurface.getPointsReferences().size());
  for (const auto &p : bezierSurface.getPointsReferences()) {
    points.emplace_back(
        *_gui._entityFactory.createPoint(p.get().getPosition()));
  }
  return points;
}