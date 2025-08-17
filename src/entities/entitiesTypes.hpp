#pragma once

#include <cstdint>
enum class EntityType : uint8_t {
  Point,
  Torus,
  Cursor,
  BezierCurveC0,
  BSplineCurve,
  InterpolatingSplineCurve,
  VirtualPoint,
  BezierSurfaceC0,
  BezierSurfaceC2,
  GregorySurface,
  Polyline,
  IntersectionCurve
};