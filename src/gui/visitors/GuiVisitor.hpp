#pragma once

#include "IEntity.hpp"
#include "IVisitor.hpp"
#include "bezierCurve.hpp"
#include <functional>
class TorusEntity;
class PointEntity;
class BezierCurveC0;
class BezierCurve;
class GUI;

class GuiVisitor : public IVisitor {
public:
  explicit GuiVisitor(GUI &gui) : _gui(gui) {}
  bool visitTorus(TorusEntity &torus) override;
  bool visitPoint(PointEntity &point) override;
  bool visitBezierCurve(BezierCurveC0 &bezierCurve) override;
  bool visitBezierCurveC2(BezierCurveC2 &bezierCurve) override;
  bool visitVirtualPoint(VirtualPoint &point) override;

private:
  GUI &_gui;
  std::vector<std::reference_wrapper<const PointEntity>> _selectedEntities;

  bool renderBasicEntitySettings(IEntity &entity);

  bool isEntitySelected(const PointEntity &entity) const;
  void unselectEntity(const PointEntity &entity);
  void selectEntity(const PointEntity &entity) {
    _selectedEntities.push_back(entity);
  }

  std::vector<std::reference_wrapper<const PointEntity>> getRemainingPoints(
      const std::vector<std::reference_wrapper<const PointEntity>> &allPoints,
      const std::vector<std::reference_wrapper<const PointEntity>>
          &currentPoints) const;

  void renderPointList(
      const std::vector<std::reference_wrapper<const PointEntity>> &entities,
      const std::string &label);

  bool renderAddingSelectedPoints(BezierCurve &bezierCurve);
  bool renderRemovingSelectedPoints(BezierCurve &bezierCurve);
};
