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
  std::vector<std::reference_wrapper<const PointEntity>>
  intersection(std::vector<std::reference_wrapper<const PointEntity>> v1,
               std::vector<std::reference_wrapper<const PointEntity>> v2);

  void selectEntity(
      int entityIndex,
      std::vector<std::reference_wrapper<const PointEntity>> &entities) {
    _selectedEntities.push_back(entities[entityIndex]);
  }

  void unselectEntity(
      int entityIndex,
      std::vector<std::reference_wrapper<const PointEntity>> &entities) {
    std::erase_if(_selectedEntities, [&](const auto &elem) {
      return &elem == &entities[entityIndex];
    });
  }
};