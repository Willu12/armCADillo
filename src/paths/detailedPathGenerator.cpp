#include "detailedPathGenerator.hpp"
#include "entitiesTypes.hpp"
#include "intersectionCurve.hpp"
#include "normalOffsetSurface.hpp"

void DetailedPathGenerator::generate() {
  for (const auto *surface : model_->surfaces()) {
    algebra::NormalOffsetSurface offset_surface{&surface->getAlgebraSurfaceC0(),
                                                cutter_->diameter_ / 2.f};
    offsetSurfaces_.push_back(offset_surface);
  }

  /// for all pairs of surfaces
  for (int i = 0; i < offsetSurfaces_.size(); ++i) {
    for (int j = i + 1; j < offsetSurfaces_.size(); ++j) {

      const auto &first_surface = offsetSurfaces_[i];
      const auto &second_surface = offsetSurfaces_[j];

      intersectionFinder_->setSurfaces(&first_surface, &second_surface);
      auto intersection = intersectionFinder_->find(false);
      if (!intersection) {
        continue; // intersections_.push_back(*intersection);
      }

      auto bounds = std::pair<std::array<algebra::Vec2f, 2>,
                              std::array<algebra::Vec2f, 2>>(
          first_surface.bounds(), second_surface.bounds());

      auto intersection_curve = std::make_unique<IntersectionCurve>(
          *intersection, bounds, intersection->looped);

      intersection_curve->setFirstPoint(intersection->firstPoint);
      intersections_.push_back(intersection_curve.get());
      scene_->addEntity(EntityType::IntersectionCurve,
                        std::move(intersection_curve));

      /*
    auto *surface_0_intersection = dynamic_cast<Intersectable
    *>(first_surface.); auto *surface_1_intersection = entities.size() == 1 ?
    surface_0_intersection : dynamic_cast<Intersectable *>(entities[1]);

    intersection_curve->getFirstTexture().setWrapping(surf0->wrapped(0),
                                                      surf0->wrapped(1));
    intersection_curve->getSecondTexture().setWrapping(surf1->wrapped(0),
                                                       surf1->wrapped(1));

    surface_0_intersection->setIntersectionTexture(
        intersection_curve->getFirstTexturePtr());
    surface_1_intersection->setIntersectionTexture(
        intersection_curve->getSecondTexturePtr());
        */
    }
  }
}