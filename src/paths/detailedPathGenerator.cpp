#include "detailedPathGenerator.hpp"
#include "entitiesTypes.hpp"
#include "intersectable.hpp"
#include "intersectionCurve.hpp"
#include "intersectionTexture.hpp"
#include "millingPath.hpp"
#include "normalOffsetSurface.hpp"
#include "vec.hpp"
#include <cstdint>
#include <queue>
#include <stdexcept>
#include <sys/types.h>

void DetailedPathGenerator::prepare() {

  //// TODO:
  //// 1. add proper interface for specyfing on which surfaces we want
  /// intersection.
  ///  2. add option to combine intersection textures.
  /// The result of this phase is that every surface of the model has proper
  /// intersection texture and can be processed

  /// for all pairs of surfaces
  for (int i = 0; i < model_->surfaces().size(); ++i) {
    for (int j = i + 1; j < model_->surfaces().size(); ++j) {

      auto *first_surface = model_->surfaces()[i];
      auto *second_surface = model_->surfaces()[j];

      auto first_offset_surface = algebra::NormalOffsetSurface(
          &first_surface->getAlgebraSurfaceC0(), cutter_->diameter_ / 2.f);

      auto second_offset_surface = algebra::NormalOffsetSurface(
          &first_surface->getAlgebraSurfaceC0(), cutter_->diameter_ / 2.f);

      intersectionFinder_->setSurfaces(&first_offset_surface,
                                       &second_offset_surface);
      auto intersection = intersectionFinder_->find(false);
      if (!intersection) {
        continue;
      }

      std::pair<std::array<algebra::Vec2f, 2>, std::array<algebra::Vec2f, 2>>
          bounds(first_surface->bounds(), second_surface->bounds());

      auto intersection_curve = std::make_unique<IntersectionCurve>(
          *intersection, bounds, intersection->looped);

      intersection_curve->setFirstPoint(intersection->firstPoint);
      intersections_.push_back(intersection_curve.get());
      scene_->addEntity(EntityType::IntersectionCurve,
                        std::move(intersection_curve));

      intersection_curve->getFirstTexture().setWrapping(
          first_surface->wrapped(0), first_surface->wrapped(1));
      intersection_curve->getSecondTexture().setWrapping(
          second_surface->wrapped(0), second_surface->wrapped(1));

      first_surface->setIntersectionTexture(
          intersection_curve->getFirstTexturePtr());
      second_surface->setIntersectionTexture(
          intersection_curve->getSecondTexturePtr());
    }
  }
}
void DetailedPathGenerator::generate() {
  /// for each surface we should have refernce to proper
  /// the part the we want to discard should be specified in the interstcion
  /// texture

  for (const auto *surface : model_->surfaces()) {
    auto starting_index = findMillingStart(*surface);
    auto milling_points = createMillingPoints(*surface, starting_index);
    auto milling_path = MillingPath{milling_points, *cutter_};
    gCodeSerializer_->serializePath(milling_path, surface->getName() + ".8f");
  }
}

DetailedPathGenerator::Coord DetailedPathGenerator::findMillingStart(
    const Intersectable &intersectableSurface) const {
  const auto &intersection_texture =
      intersectableSurface.getIntersectionTexture();
  const auto size = intersection_texture.getSize();

  for (int y = 0; y < size.height; ++y) {
    for (int x = 0; x < size.width; ++x) {
      if (intersection_texture.getCellType(x, y) ==
          IntersectionTexture::CellType::Keep) {
        return {.x = x, .y = y};
      }
    }
  }

  throw std::runtime_error("Failed to find detailed milling starting point");
}

std::vector<algebra::Vec3f>
DetailedPathGenerator::createMillingPoints(const BezierSurface &surface,
                                           Coord startingIndex) const {
  const auto &intersection_texture = surface.getIntersectionTexture();
  const auto size = intersection_texture.getSize();

  const auto &offset_surface = algebra::NormalOffsetSurface(
      &surface.getAlgebraSurfaceC0(), cutter_->diameter_ / 2.f);

  std::array<Coord, 4> directions = {{{.x = 1, .y = 0},
                                      {.x = -1, .y = 0},
                                      {.x = 0, .y = 1},
                                      {.x = 0, .y = -1}}};

  std::vector<bool> visited(size.height * size.width, false);
  std::queue<Coord> queue;

  auto global_index = [&size](Coord index) {
    return index.y * size.width + index.x;
  };

  auto uv = [&size](const Coord &index) -> algebra::Vec2f {
    float u = static_cast<float>(index.x) / static_cast<float>(size.width);
    float v = static_cast<float>(index.y) / static_cast<float>(size.height);
    return {u, v};
  };

  queue.push(startingIndex);
  visited[global_index(startingIndex)] = true;
  std::vector<algebra::Vec3f> milling_points;

  while (!queue.empty()) {
    auto coord = queue.front();
    queue.pop();

    milling_points.push_back(offset_surface.value(uv(coord)));

    for (const auto &direction : directions) {
      if (coord.x + direction.x < 0 || coord.x + direction.x >= size.width ||
          coord.y + direction.y < 0 || coord.y + direction.y >= size.height) {
        continue;
      }

      Coord next_coord = {.x = coord.x + direction.x,
                          .y = coord.y + direction.y};

      if (intersection_texture.getCellType(coord.x, coord.y) !=
          IntersectionTexture::CellType::Keep) {
        continue;
      }

      if (!visited[global_index(next_coord)]) {
        queue.push(next_coord);
        visited[global_index(next_coord)] = true;
      }
    }
  }
}