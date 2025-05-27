#pragma once

#include "IEntity.hpp"
#include "IGroupedEntity.hpp"
#include "IVisitor.hpp"
#include "nlohmann/json.hpp"
class IEntity;
class JsonSerializer : IVisitor {
public:
  bool visitTorus(TorusEntity &torus) override;
  bool visitPoint(PointEntity &point) override;
  bool visitBezierCurve(BezierCurveC0 &bezierCurve) override;
  bool visitBSplineCurve(BSplineCurve &bSplineCurve) override;
  bool visitVirtualPoint(VirtualPoint &point) override;
  bool visitInterpolatingSplineCurve(
      InterpolatingSplineC2 &interpolatingSpline) override;
  bool visitBezierSurface(BezierSurface &bezierSurface) override;
  bool visitBezierSurfaceC0(BezierSurfaceC0 &bezierSurface) override;
  bool visitBezierSurfaceC2(BezierSurfaceC2 &bezierSurface) override;

private:
  void serializePosition(nlohmann::json &j, const IEntity &entity);
  void serializeRotation(nlohmann::json &j, const IEntity &entity);
  void serializeScale(nlohmann::json &j, const IEntity &entity);
  void serializeControlPoints(nlohmann::json &j,
                              const IGroupedEntity &groupedEntity);
};