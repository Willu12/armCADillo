#pragma once

#include "GCodeSerializer.hpp"
#include "bezierSurface.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "intersectable.hpp"
#include "intersectionCurve.hpp"
#include "intersectionFinder.hpp"
#include "model.hpp"
#include "normalOffsetSurface.hpp"
#include "scene.hpp"
#include "vec.hpp"
#include <cstdint>
#include <vector>
class DetailedPathGenerator {
public:
  struct Coord {
    int x;
    int y;
  };
  void prepare();
  void generate();
  void setModel(const Model *model) { model_ = model; }
  void setCutter(const Cutter *cutter) { cutter_ = cutter; }
  void setScene(Scene *scene) { scene_ = scene; }
  void setIntersectionFinder(IntersectionFinder *intersectionFinder) {
    intersectionFinder_ = intersectionFinder;
  }
  void setHeightMap(const HeightMap *heightMap) { heightMap_ = heightMap; }

  const std::vector<IntersectionCurve *> &intersections() const {
    return intersections_;
  }

private:
  const Model *model_ = nullptr;
  const Cutter *cutter_ = nullptr;
  const HeightMap *heightMap_ = nullptr;
  const GCodeSerializer *gCodeSerializer_ = nullptr; // remove later
  Scene *scene_ = nullptr;
  std::vector<IntersectionCurve *> intersections_;
  IntersectionFinder *intersectionFinder_;

  std::vector<algebra::Vec3f> createMillingPoints(const BezierSurface &surface,
                                                  Coord startingIndex) const;

  Coord findMillingStart(const Intersectable &intersectableSurface) const;
};