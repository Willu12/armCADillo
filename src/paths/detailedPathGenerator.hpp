#pragma once

#include "cutter.hpp"
#include "intersectionCurve.hpp"
#include "intersectionFinder.hpp"
#include "model.hpp"
#include "normalOffsetSurface.hpp"
#include "scene.hpp"
#include <vector>
class DetailedPathGenerator {
public:
  void generate();
  void setModel(const Model *model) { model_ = model; }
  void setCutter(const Cutter *cutter) { cutter_ = cutter; }
  void setScene(Scene *scene) { scene_ = scene; }
  void setIntersectionFinder(IntersectionFinder *intersectionFinder) {
    intersectionFinder_ = intersectionFinder;
  }

  const std::vector<IntersectionCurve *> &intersections() const {
    return intersections_;
  }

private:
  const Model *model_ = nullptr;
  const Cutter *cutter_ = nullptr;
  Scene *scene_ = nullptr;
  std::vector<algebra::NormalOffsetSurface> offsetSurfaces_;
  std::vector<IntersectionCurve *> intersections_;
  IntersectionFinder *intersectionFinder_;
};