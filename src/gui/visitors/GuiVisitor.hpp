#pragma once

#include "IEntity.hpp"
#include "IVisitor.hpp"
#include <functional>
class TorusEntity;
class PointEntity;
class BezierCurve;
class GUI;

class GuiVisitor : public IVisitor {
public:
  explicit GuiVisitor(GUI &gui) : _gui(gui) {}
  virtual bool visitTorus(TorusEntity &torus) override;
  virtual bool visitPoint(PointEntity &point) override;
  virtual bool visitBezierCurve(BezierCurve &bezierCurve) override;

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
