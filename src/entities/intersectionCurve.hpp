#pragma once

#include "IEntity.hpp"
#include "intersectionFinder.hpp"
#include "polyline.hpp"
#include "texture.hpp"
#include "vec.hpp"
#include <memory>
#include <ranges>

class IntersectionTexture;
class IntersectionCurve : public IEntity {
public:
  explicit IntersectionCurve(const Intersection &intersection);
  void updateMesh() override { polyline_->updateMesh(); }
  const IMeshable &getMesh() const override { return polyline_->getMesh(); };
  const IntersectionTexture &getFirstTexture() const { return *texture0_; }
  const IntersectionTexture &getSecondTexture() const { return *texture1_; }
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitIntersectionCurve(*this);
  }

private:
  inline static int kClassId = 0;

  std::unique_ptr<Polyline> polyline_;
  std::unique_ptr<IntersectionTexture> texture0_;
  std::unique_ptr<IntersectionTexture> texture1_;
};