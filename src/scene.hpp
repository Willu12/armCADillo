#pragma once
#include "IEntity.hpp"

#include "camera.hpp"
#include "entitiesTypes.hpp"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

class Scene {
public:
  explicit Scene(std::shared_ptr<Camera> camera) : _camera(std::move(camera)){};

  std::vector<std::shared_ptr<IEntity>> getEntites();
  void addEntity(EntityType entityType, const std::shared_ptr<IEntity> &entity);
  std::shared_ptr<Camera> getCamera();
  void removeEntities(std::vector<std::shared_ptr<IEntity>> &entitiesToRemove);
  const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>> &
  getGroupedEntities() const;
  std::vector<std::shared_ptr<IEntity>> getPickables() const;
  std::vector<std::shared_ptr<IEntity>> getVirtualPoints() const;
  std::vector<std::shared_ptr<IEntity>> getPoints() const;

private:
  std::shared_ptr<Camera> _camera;
  std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>>
      _entities;

  void filterEntitiesToRemove(
      std::vector<std::shared_ptr<IEntity>> &entitiesToRemove);

  void enqueueSurfacePoints(
      std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) const;
};