#include "intersectionCurve.hpp"

#include "texture.hpp"

IntersectionCurve::IntersectionCurve(const Intersection &intersection) {
  _id = kClassId++;
  _name = "IntersectionCurve" + std::to_string(_id);
  std::vector<algebra::Vec3f> points(intersection.points.size());
  for (const auto &[u, p] : intersection.points | std::views::enumerate)
    points[u] = p.point;
  polyline_ = std::make_unique<Polyline>(points);

  auto textures = IntersectionTexture::createIntersectionTextures(intersection);
  texture0_ = std::move(textures.first);
  texture1_ = std::move(textures.second);
}