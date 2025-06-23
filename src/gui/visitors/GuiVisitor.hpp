#pragma once

#include "IEntity.hpp"
#include "IVisitor.hpp"
#include "bezierCurve.hpp"
#include "bezierSurface.hpp"
#include "gregorySurface.hpp"
#include "interpolatingSplineC2.hpp"
#include <functional>
#include <vector>
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
  bool visitBSplineCurve(BSplineCurve &bezierCurve) override;
  bool visitVirtualPoint(VirtualPoint &point) override;
  bool visitInterpolatingSplineCurve(
      InterpolatingSplineC2 &interpolatingSpline) override;
  bool visitBezierSurface(BezierSurface &bezierSurface) override;
  bool visitBezierSurfaceC0(BezierSurfaceC0 &bezierSurface) override;
  bool visitBezierSurfaceC2(BezierSurfaceC2 &bezierSurface) override;
  bool visitGregorySurface(GregorySurface &gregorySurface) override;
  bool visitIntersectionCurve(IntersectionCurve &intersectionCurve) override;

private:
  GUI &_gui;
  std::vector<std::reference_wrapper<PointEntity>> _selectedEntities;
  std::vector<std::reference_wrapper<const VirtualPoint>>
      _selectedVirtualPoints;

  bool renderBasicEntitySettings(IEntity &entity);

  bool isEntitySelected(const PointEntity &entity) const;
  void unselectEntity(const PointEntity &entity);
  void selectEntity(PointEntity &entity);

  std::vector<std::reference_wrapper<PointEntity>> getRemainingPoints(
      const std::vector<std::reference_wrapper<PointEntity>> &allPoints,
      const std::vector<std::reference_wrapper<PointEntity>> &currentPoints)
      const;

  void renderPointList(
      const std::vector<std::reference_wrapper<PointEntity>> &entities,
      const std::string &label);

  bool renderAddingSelectedPoints(BezierCurve &bezierCurve);
  bool renderRemovingSelectedPoints(BezierCurve &bezierCurve);

  // Virtual Points Handlers

  void renderVirtualPointList(
      const std::vector<std::shared_ptr<VirtualPoint>> &virtualPoints);
  bool isVirtualPointSelected(const VirtualPoint &point) const;
  void selectVirtualPoint(const VirtualPoint &point);
  void unselectVirtualPoint(const VirtualPoint &point);

  bool renderCurveGui(BezierCurve &curve);
};
