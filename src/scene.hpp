#pragma once
#include "IEntity.hpp"
#include "bSplineCurve.hpp"
#include "bezierSurface.hpp"
#include "camera.hpp"
#include "entitiesTypes.hpp"
#include "pointEntity.hpp"
#include "virtualPoint.hpp"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

class Scene {
public:
  Scene(std::shared_ptr<Camera> camera) : _camera(camera){};

  std::vector<std::shared_ptr<IEntity>> getEntites() {
    std::vector<std::shared_ptr<IEntity>> entities;
    for (const auto &specificEntities : _entities) {
      entities.insert(entities.end(), specificEntities.second.begin(),
                      specificEntities.second.end());
    }
    return entities;
  }

  void addEntity(EntityType entityType,
                 const std::shared_ptr<IEntity> &entity) {
    _entities[entityType].emplace_back(entity);
  }

  std::shared_ptr<Camera> getCamera() { return _camera; }
  void removeEntities(std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) {

    filterEntitiesToRemove(entitiesToRemove);
    EnqueueSurfacePoints(entitiesToRemove);
    for (auto it = _entities.begin(); it != _entities.end();) {
      auto &vec = it->second;

      auto new_end = std::ranges::remove_if(vec, [&](const auto &e) {
        return std::ranges::find(entitiesToRemove, e) != entitiesToRemove.end();
      });
      vec.erase(new_end.begin(), vec.end());

      if (vec.empty())
        it = _entities.erase(it);
      else
        ++it;
    }
  }

  const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>> &
  getGroupedEntities() const {
    return _entities;
  }

  std::vector<std::shared_ptr<IEntity>> getPickables() const {
    auto points = getPoints();
    auto vPoints = getVirtualPoints();
    points.insert(points.begin(), vPoints.begin(), vPoints.end());
    return points;
  }

  std::vector<std::shared_ptr<IEntity>> getVirtualPoints() const {
    std::vector<std::shared_ptr<IEntity>> virtualPoints;
    if (!_entities.contains(EntityType::BSplineCurve))
      return virtualPoints;
    for (const auto &entity : _entities.at(EntityType::BSplineCurve)) {
      auto bezierCurve = std::dynamic_pointer_cast<BSplineCurve>(entity);
      if (!bezierCurve || !bezierCurve->showBezierPoints())
        continue;

      const auto &vPoints = bezierCurve->getVirtualPoints();
      virtualPoints.insert(virtualPoints.end(), vPoints.begin(), vPoints.end());
    }
    return virtualPoints;
  }

  std::vector<std::shared_ptr<IEntity>> getPoints() const {
    if (!_entities.contains(EntityType::Point))
      return std::vector<std::shared_ptr<IEntity>>();
    return _entities.at(EntityType::Point);
  }

private:
  std::shared_ptr<Camera> _camera;
  std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>>
      _entities;

  void filterEntitiesToRemove(
      std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) {
    std::erase_if(entitiesToRemove, [](const std::shared_ptr<IEntity> &entity) {
      if (auto point = std::dynamic_pointer_cast<PointEntity>(entity)) {
        return point->surfacePoint();
      }
      return false;
    });
  }

  void EnqueueSurfacePoints(
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
          if (it != points.end())
            entitiesToRemove.push_back(*it);
        }
      }
    }
  }
};