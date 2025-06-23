#pragma once

#include "IEntity.hpp"
#include "intersectionFinder.hpp"
#include "polyline.hpp"
#include "vec.hpp"
#include <ranges>
class IntersectionCurve : public IEntity {
public:
  explicit IntersectionCurve(const Intersection &intersection) {
    _id = kClassId++;
    _name = "IntersectionCurve" + std::to_string(_id);
    std::vector<algebra::Vec3f> points(intersection.points.size());
    for (const auto &[u, p] : intersection.points | std::views::enumerate)
      points[u] = p.point;
    polyline_ = std::make_unique<Polyline>(points);

    // craete texture 1
    // craete texture 2
  }
  void updateMesh() override { polyline_->updateMesh(); }
  const IMeshable &getMesh() const override { return polyline_->getMesh(); };

private:
  inline static int kClassId = 0;

  std::unique_ptr<Polyline> polyline_;
  // texutre 1
  // texture 2
};