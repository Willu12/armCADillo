#pragma once

class TorusEntity;
class PointEntity;
class BezierCurveC0;
class BezierCurveC2;

class IVisitor {
public:
  ~IVisitor() = default;
  virtual bool visitTorus(TorusEntity &torus) = 0;
  virtual bool visitPoint(PointEntity &point) = 0;
  virtual bool visitBezierCurve(BezierCurveC0 &bezierCurve) = 0;
  virtual bool visitBezierCurveC2(BezierCurveC2 &bezierCurve) = 0;
};