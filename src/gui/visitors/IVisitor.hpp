#pragma once

class VirtualPoint;
class TorusEntity;
class PointEntity;
class BezierCurveC0;
class BSplineCurve;

class IVisitor {
public:
  virtual ~IVisitor() = default;
  virtual bool visitTorus(TorusEntity &torus) = 0;
  virtual bool visitPoint(PointEntity &point) = 0;
  virtual bool visitVirtualPoint(VirtualPoint &point) = 0;
  virtual bool visitBezierCurve(BezierCurveC0 &bezierCurve) = 0;
  virtual bool visitBSplineCurve(BSplineCurve &bezierCurve) = 0;
};