#pragma once

#include "IEntity.hpp"
#include "intersectionFinder.hpp"
#include "intersectionTexture.hpp"
#include "point.hpp"
#include "polyline.hpp"
#include "texture.hpp"
#include "vec.hpp"
#include <memory>
#include <ranges>

class IntersectionCurve : public IEntity {
public:
  explicit IntersectionCurve(
      const Intersection &intersection,
      std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>
          bounds,
      bool looped);
  void updateMesh() override { polyline_->updateMesh(); }
  const IMeshable &getMesh() const override { return polyline_->getMesh(); };
  IntersectionTexture &getFirstTexture() { return *texture0_; }
  IntersectionTexture &getSecondTexture() { return *texture1_; }
  IntersectionTexture *getFirstTexturePtr() const { return texture0_.get(); }
  IntersectionTexture *getSecondTexturePtr() const { return texture1_.get(); }
  const Polyline &getPolyline() const { return *polyline_; }
  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitIntersectionCurve(*this);
  }

  const bool &isLooped() const { return looped_; }
  bool &isDead() { return dead_; }
  void setFirstPoint(const algebra::Vec3f &pos) {
    firstPoint_ = std::make_unique<Point>(pos);
  }
  Point &firstPoint() const { return *firstPoint_; };

private:
  inline static int kClassId = 0;
  std::unique_ptr<Point> firstPoint_;
  bool dead_ = false;
  bool looped_ = false;
  std::unique_ptr<Polyline> polyline_;
  std::unique_ptr<IntersectionTexture> texture0_;
  std::unique_ptr<IntersectionTexture> texture1_;
};