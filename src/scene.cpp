#include "scene.hpp"
#include "IEntity.hpp"
#include "IGroupedEntity.hpp"
#include "bSplineCurve.hpp"
#include "bezierSurface.hpp"
#include "entitiesTypes.hpp"
#include "gregorySurface.hpp"
#include "intersectionCurve.hpp"
#include "pointEntity.hpp"
#include <algorithm>
#include <functional>
#include <memory>

std::vector<std::shared_ptr<IEntity>> Scene::getEntites() const {
  std::vector<std::shared_ptr<IEntity>> entities;
  for (const auto &specificEntities : _entities) {
    entities.insert(entities.end(), specificEntities.second.begin(),
                    specificEntities.second.end());
  }
  return entities;
}

void Scene::addEntity(EntityType entityType,
                      const std::shared_ptr<IEntity> &entity) {
  _entities[entityType].emplace_back(entity);
}

std::shared_ptr<Camera> Scene::getCamera() { return _camera; }
void Scene::removeEntities(
    std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) {

  filterEntitiesToRemove(entitiesToRemove);
  enqueueSurfacePoints(entitiesToRemove);
  for (auto it = _entities.begin(); it != _entities.end();) {
    auto &vec = it->second;

    auto new_end = std::ranges::remove_if(vec, [&](const auto &e) {
      return std::ranges::find(entitiesToRemove, e) != entitiesToRemove.end();
    });
    vec.erase(new_end.begin(), vec.end());

    if (vec.empty()) {
      it = _entities.erase(it);
    } else {
      ++it;
    }
  }
}

const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>> &
Scene::getGroupedEntities() const {
  return _entities;
}

std::vector<std::shared_ptr<IEntity>> Scene::getPickables() const {
  auto points = getPoints();
  auto vPoints = getVirtualPoints();
  points.insert(points.begin(), vPoints.begin(), vPoints.end());

  return points;
}

std::vector<std::shared_ptr<IEntity>> Scene::getVirtualPoints() const {
  std::vector<std::shared_ptr<IEntity>> virtualPoints;
  if (!_entities.contains(EntityType::BSplineCurve)) {
    return virtualPoints;
  }

  for (const auto &entity : _entities.at(EntityType::BSplineCurve)) {
    auto bezierCurve = std::dynamic_pointer_cast<BSplineCurve>(entity);
    if (!bezierCurve || !bezierCurve->showBezierPoints()) {
      continue;
    }

    const auto &vPoints = bezierCurve->getVirtualPoints();
    virtualPoints.insert(virtualPoints.end(), vPoints.begin(), vPoints.end());
  }

  return virtualPoints;
}

std::vector<std::shared_ptr<IEntity>> Scene::getPoints() const {
  if (!_entities.contains(EntityType::Point)) {
    return std::vector<std::shared_ptr<IEntity>>();
  }

  return _entities.at(EntityType::Point);
}

void Scene::filterEntitiesToRemove(
    std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) {
  std::erase_if(entitiesToRemove, [](const std::shared_ptr<IEntity> &entity) {
    if (auto point = std::dynamic_pointer_cast<PointEntity>(entity)) {
      return point->surfacePoint();
    }
    return false;
  });
}

void Scene::enqueueSurfacePoints(
    std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) const {
  const auto &points = getPoints();

  for (const auto &entity : entitiesToRemove) {
    if (auto surface = std::dynamic_pointer_cast<BezierSurface>(entity)) {
      const auto &surfacePoints = surface->getPoints();
      for (const auto &surfacePoint : surfacePoints) {
        const auto it =
            std::ranges::find_if(points, [&surfacePoint](const auto &point) {
              return point->getId() == surfacePoint.get().getId();
            });
        if (it != points.end()) {
          entitiesToRemove.push_back(*it);
        }
      }
    }
  }
}

void Scene::enqueueDeadGregoryPatches() {
  if (!_entities.contains(EntityType::GregorySurface)) {
    return;
  }

  for (const auto &entity : _entities.at(EntityType::GregorySurface)) {
    auto gregory = std::dynamic_pointer_cast<GregorySurface>(entity);
    if (gregory->isDead()) {
      _deadEntities.push_back(gregory);
    }
  }

  if (!_entities.contains(EntityType::IntersectionCurve)) {
    return;
  }

  for (const auto &entity : _entities.at(EntityType::IntersectionCurve)) {
    auto gregory = std::dynamic_pointer_cast<IntersectionCurve>(entity);
    if (gregory->isDead()) {
      _deadEntities.push_back(gregory);
    }
  }
}

std::shared_ptr<IEntity> Scene::contractEdge(const PointEntity &p1,
                                             const PointEntity &p2) {
  const auto avgPos = (p1.getPosition() + p2.getPosition()) / 2.f;
  const auto centerPoint = std::make_shared<PointEntity>(avgPos);
  addEntity(EntityType::Point, centerPoint);

  rebindReferences(p1, *centerPoint);
  rebindReferences(p2, *centerPoint);
  auto &points = _entities.at(EntityType::Point);
  std::erase_if(points, [&p1, &p2](const auto &p) {
    return p->getId() == p1.getId() || p->getId() == p2.getId();
  });
  return centerPoint;
}

void Scene::rebindReferences(const PointEntity &oldPoint,
                             PointEntity &newPoint) {
  for (auto &pair : _entities) {
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
      auto groupedEntity = std::dynamic_pointer_cast<IGroupedEntity>(entitity);
      if (groupedEntity == nullptr) {
        return;
      }
      auto &points = groupedEntity->getPointsReferences();
      for (auto &pointRef : points) {
        if (pointRef.get().getId() == oldPoint.getId()) {
          pointRef = std::ref(newPoint);
        }
      }
    }
  }
}

void Scene::removeDeadEntities() {
  enqueueDeadGregoryPatches();
  removeEntities(_deadEntities);
  _deadEntities.clear();
}