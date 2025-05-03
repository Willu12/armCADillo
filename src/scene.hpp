#pragma once
#include "IEntity.hpp"
#include "bezierCurveC2.hpp"
#include "camera.hpp"
#include "entitiesTypes.hpp"
#include "virtualPoints.hpp"
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
  void removeEntities(
      const std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) {
    for (auto it = _entities.begin(); it != _entities.end();) {
      auto &vec = it->second;

      vec.erase(std::remove_if(vec.begin(), vec.end(),
                               [&](const std::shared_ptr<IEntity> &e) {
                                 return std::find(entitiesToRemove.begin(),
                                                  entitiesToRemove.end(),
                                                  e) != entitiesToRemove.end();
                               }),
                vec.end());

      if (vec.empty())
        it = _entities.erase(it);
      else
        ++it;
    }
  }

  const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>> &
  getGroupedEntities() {
    return _entities;
  }

  std::vector<std::shared_ptr<IEntity>> getPoints() {
    if (!_entities.contains(EntityType::Point))
      return std::vector<std::shared_ptr<IEntity>>();
    return _entities.at(EntityType::Point);
  }
  std::vector<std::shared_ptr<IEntity>> getVirtualPoints() const {
    std::vector<std::shared_ptr<IEntity>> virtualPoints;
    if (!_entities.contains(EntityType::BezierCurveC2))
      return virtualPoints;
    for (const auto &entity : _entities.at(EntityType::BezierCurveC2)) {
      auto bezierCurve = std::dynamic_pointer_cast<BezierCurveC2>(entity);
      if (!bezierCurve || !bezierCurve->showBezierPoints())
        continue;

      const auto &vPoints = bezierCurve->getVirtualPoints();
      virtualPoints.insert(virtualPoints.end(), vPoints.begin(), vPoints.end());
    }
    return virtualPoints;
  }

private:
  std::shared_ptr<Camera> _camera;
  std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>>
      _entities;
};