#pragma once

class VirtualPoint;
class TorusEntity;
class PointEntity;
class BezierCurveC0;
class BSplineCurve;
class InterpolatingSplineC2;
class BezierSurface;
class BezierSurfaceC0;
class BezierSurfaceC2;
class GregorySurface;

class IVisitor {
public:
  virtual ~IVisitor() = default;
  virtual bool visitTorus(TorusEntity &torus) = 0;
  virtual bool visitPoint(PointEntity &point) = 0;
  virtual bool visitVirtualPoint(VirtualPoint &point) = 0;
  virtual bool visitBezierCurve(BezierCurveC0 &bezierCurve) = 0;
  virtual bool visitBSplineCurve(BSplineCurve &bezierCurve) = 0;
  virtual bool
  visitInterpolatingSplineCurve(InterpolatingSplineC2 &interpolatingSpline) = 0;
  virtual bool visitBezierSurface(BezierSurface &bezierSurface) = 0;
  virtual bool visitBezierSurfaceC0(BezierSurfaceC0 &bezierSurfaceC0) = 0;
  virtual bool visitBezierSurfaceC2(BezierSurfaceC2 &bezierSurfaceC2) = 0;
  virtual bool visitGregorySurface(GregorySurface &gregorySurface) = 0;
};