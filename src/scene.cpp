#include "scene.hpp"
#include "IEntity.hpp"
#include "IGroupedEntity.hpp"
#include "ISubscriber.hpp"
#include "bSplineCurve.hpp"
#include "bezierSurface.hpp"
#include "camera.hpp"
#include "entitiesTypes.hpp"
#include "gregorySurface.hpp"
#include "intersectionCurve.hpp"
#include "pointEntity.hpp"
#include <algorithm>
#include <functional>
#include <memory>

std::vector<IEntity *> Scene::getEntites() const {
  std::vector<IEntity *> entities;
  for (const auto &specific_entities : entities_) {
    for (const auto &specifc_entity : specific_entities.second) {
      entities.push_back(specifc_entity.get());
    }
  }
  return entities;
}

void Scene::addEntity(EntityType entityType, std::unique_ptr<IEntity> entity) {
  entities_[entityType].emplace_back(std::move(entity));
}

Camera *Scene::getCamera() { return camera_; }

void Scene::removeEntities(std::vector<IEntity *> &entitiesToRemove) {
  if (entitiesToRemove.empty()) {
    return;
  }

  filterEntitiesToRemove(entitiesToRemove);
  enqueueSurfacePoints(entitiesToRemove);
  for (auto it = entities_.begin(); it != entities_.end();) {
    auto &vec = it->second;

    auto new_end = std::ranges::remove_if(vec, [&](const auto &e) {
      return std::ranges::find(entitiesToRemove, e.get()) !=
             entitiesToRemove.end();
    });
    vec.erase(new_end.begin(), vec.end());

    if (vec.empty()) {
      it = entities_.erase(it);
    } else {
      ++it;
    }
  }
}

std::unordered_map<EntityType, std::vector<IEntity *>>
Scene::getGroupedEntities() const {
  std::unordered_map<EntityType, std::vector<IEntity *>> grouped_entities;

  for (const auto &pair : entities_) {
    std::vector<IEntity *> specific_entities;
    specific_entities.reserve(pair.second.size());

    for (const auto &entity : pair.second) {
      specific_entities.push_back(entity.get());
    }
    grouped_entities.insert({pair.first, specific_entities});
  }

  return grouped_entities;
}

std::vector<IEntity *> Scene::getPickables() const {
  auto points = getPoints();
  auto virtual_points = getVirtualPoints();
  points.insert(points.begin(), virtual_points.begin(), virtual_points.end());

  return points;
}

std::vector<IEntity *> Scene::getVirtualPoints() const {
  std::vector<IEntity *> virtual_points;
  if (!entities_.contains(EntityType::BSplineCurve)) {
    return virtual_points;
  }

  for (const auto &entity : entities_.at(EntityType::BSplineCurve)) {
    auto *b_spline = dynamic_cast<BSplineCurve *>(entity.get());
    if (!b_spline || !b_spline->showBezierPoints()) {
      continue;
    }

    const auto &bezier_virtual_points = b_spline->getVirtualPoints();
    virtual_points.insert(virtual_points.end(), bezier_virtual_points.begin(),
                          bezier_virtual_points.end());
  }

  return virtual_points;
}

std::vector<IEntity *> Scene::getPoints() const {
  std::vector<IEntity *> points;
  if (!entities_.contains(EntityType::Point)) {
    return points;
  }

  points.reserve(entities_.at(EntityType::Point).size());
  for (const auto &point : entities_.at(EntityType::Point)) {
    points.push_back(point.get());
  }

  return points;
}

void Scene::filterEntitiesToRemove(std::vector<IEntity *> &entitiesToRemove) {
  std::erase_if(entitiesToRemove, [](const IEntity *entity) {
    if (const auto *point = dynamic_cast<const PointEntity *>(entity)) {
      return point->surfacePoint();
    }
    return false;
  });
}

void Scene::enqueueSurfacePoints(
    std::vector<IEntity *> &entitiesToRemove) const {
  const auto &points = getPoints();
  std::vector<IEntity *> points_to_remove;

  for (const auto &entity : entitiesToRemove) {
    if (auto *surface = dynamic_cast<BezierSurface *>(entity)) {
      const auto &surface_points = surface->getPoints();
      for (const auto &surface_point : surface_points) {
        const auto it =
            std::ranges::find_if(points, [&surface_point](const auto &point) {
              return point->getId() == surface_point.get().getId();
            });

        if (it != points.end()) {
          points_to_remove.push_back(*it);
        }
      }
    }
  }

  entitiesToRemove.insert(entitiesToRemove.end(), points_to_remove.begin(),
                          points_to_remove.end());
}

/// i dont like this function
/// it should be changed to somekind of clearDeadEntities()?
/// 1. entities could have dead flag and that be removed
/// 2. entities could somehow be added to killing queue
///
void Scene::enqueueDeadGregoryPatches() {
  if (!entities_.contains(EntityType::GregorySurface)) {
    return;
  }

  for (const auto &entity : entities_.at(EntityType::GregorySurface)) {
    auto *gregory = dynamic_cast<GregorySurface *>(entity.get());
    if (gregory->isDead()) {
      deadEntities_.push_back(gregory);
    }
  }

  if (!entities_.contains(EntityType::IntersectionCurve)) {
    return;
  }

  for (const auto &entity : entities_.at(EntityType::IntersectionCurve)) {
    auto *gregory = dynamic_cast<IntersectionCurve *>(entity.get());
    if (gregory->isDead()) {
      deadEntities_.push_back(gregory);
    }
  }
}

IEntity *Scene::contractEdge(const PointEntity &p1, const PointEntity &p2) {
  const auto avg_pos = (p1.getPosition() + p2.getPosition()) / 2.f;
  auto center_point = std::make_unique<PointEntity>(avg_pos);

  PointEntity *center_point_ptr = center_point.get();
  addEntity(EntityType::Point, std::move(center_point));

  rebindReferences(p1, *center_point_ptr);
  rebindReferences(p2, *center_point_ptr);
  auto &points = entities_.at(EntityType::Point);
  std::erase_if(points, [&p1, &p2](const auto &p) {
    return p->getId() == p1.getId() || p->getId() == p2.getId();
  });

  return center_point_ptr;
}

void Scene::rebindReferences(const PointEntity &oldPoint,
                             PointEntity &newPoint) {
  for (auto &pair : entities_) {
    if (pair.first == EntityType::Point || pair.first == EntityType::Torus ||
        pair.first == EntityType::GregorySurface) {
      continue;
    }

    for (const auto &sub : oldPoint.getSubscribers()) {
      sub.get().subscribe(newPoint);
      sub.get().unsubscribe(oldPoint);
    }
    auto &entities = pair.second;
    for (auto &entitity : entities) {
      auto *grouped_entity = dynamic_cast<IGroupedEntity *>(entitity.get());

      if (grouped_entity == nullptr) {
        return;
      }

      auto &points = grouped_entity->getPointsReferences();
      for (auto &point_ref : points) {
        if (point_ref.get().getId() == oldPoint.getId()) {
          point_ref = std::ref(newPoint);
        }
      }
    }
  }
}

void Scene::removeDeadEntities() {
  enqueueDeadGregoryPatches();
  removeEntities(deadEntities_);
  deadEntities_.clear();
}

void Scene::updateDirtyEntities() {
  for (const auto &entity_type_vector_pair : entities_) {
    for (const auto &entity : entity_type_vector_pair.second) {
      if (entity->dirty()) {
        if (auto *subscriber = dynamic_cast<ISubscriber *>(entity.get())) {
          subscriber->update();
        }
      }
    }
  }
}

void Scene::processFrame() {
  removeDeadEntities();
  updateDirtyEntities();
}