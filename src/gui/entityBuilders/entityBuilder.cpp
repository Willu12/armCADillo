#include "gui.hpp"

#include "entityBuilders/bezierCurveC0Builder.hpp"
#include "entityBuilders/bezierCurveC2Builder.hpp"
#include "entityBuilders/bezierSurfaceC0Builder.hpp"
#include "entityBuilders/gregoryPatchBuilder.hpp"
#include "entityBuilders/interpolatingSplineBuilder.hpp"
#include "entityBuilders/polylineBuilder.hpp"
#include "pointBuilder.hpp"
#include "torusBuilder.hpp"

std::optional<IEntity *> PointBuilder::create() const {
  return factory_->createPoint(gui_->getCursorPosition());
};

std::optional<IEntity *> TorusBuilder::create() const {
  return factory_->createTorus(gui_->getCursorPosition());
};

std::optional<IEntity *> BezierCurveC0Builder::create() const {
  return factory_->createBezierCurveC0(gui_->getSelectedPoints());
};

std::optional<IEntity *> BSplineCurveBuilder::create() const {
  return factory_->createBSplineCurve(gui_->getSelectedPoints());
};

std::optional<IEntity *> BezierSurfaceC2Builder::create() const {
  return factory_->createBezierSurfaceC2(gui_->getCursorPosition(), uPatches_,
                                         vPatches_, x_, y_, cyllinder_ != 0);
};

std::optional<IEntity *> BezierSurfaceC0Builder::create() const {
  return factory_->createBezierSurfaceC0(gui_->getCursorPosition(), uPatches_,
                                         vPatches_, x_, y_, cyllinder_ != 0);
};

std::optional<IEntity *> GregoryPatchBuilder::create() const {
  return factory_->createGregoryPatch(gui_->getSelectedSurfacesC0());
};

std::optional<IEntity *> InterpolatingSplineBuilder::create() const {
  return factory_->createInterpolatingSpline(gui_->getSelectedPoints());
};

std::optional<IEntity *> PolylineBuilder::create() const {
  auto selectedPoints = gui_->getSelectedPoints();
  std::vector<algebra::Vec3f> pointsPositions(selectedPoints.size());

  for (const auto &[i, point] : selectedPoints | std::views::enumerate) {
    pointsPositions[i] = point.get().getPosition();
  }

  return factory_->createPolyline(pointsPositions);
};