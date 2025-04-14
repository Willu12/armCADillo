#pragma once

class TorusEntity;
class PointEntity;
class BezierCurve;

class IVisitor {
public:
  ~IVisitor() = default;
  virtual bool visitTorus(TorusEntity &torus) = 0;
  virtual bool visitPoint(PointEntity &point) = 0;
  virtual bool visitBezierCurve(BezierCurve &bezierCurve) = 0;
};