#pragma once
#include "IEntity.hpp"

#include "camera.hpp"
#include "entitiesTypes.hpp"
#include "pointEntity.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class Scene {
public:
  explicit Scene(Camera *camera) : _camera(camera){};

  std::vector<std::shared_ptr<IEntity>> getEntites() const;
  void addEntity(EntityType entityType, const std::shared_ptr<IEntity> &entity);
  Camera *getCamera();
  void removeEntities(std::vector<std::shared_ptr<IEntity>> &entitiesToRemove);
  const std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>> &
  getGroupedEntities() const;
  std::vector<std::shared_ptr<IEntity>> getPickables() const;
  std::vector<std::shared_ptr<IEntity>> getVirtualPoints() const;
  std::vector<std::shared_ptr<IEntity>> getPoints() const;

  void processFrame();
  friend class GUI;

private:
  Camera *_camera;
  std::unordered_map<EntityType, std::vector<std::shared_ptr<IEntity>>>
      _entities;
  std::vector<std::shared_ptr<IEntity>> _deadEntities;

  void filterEntitiesToRemove(
      std::vector<std::shared_ptr<IEntity>> &entitiesToRemove);

  void enqueueSurfacePoints(
      std::vector<std::shared_ptr<IEntity>> &entitiesToRemove) const;
  void enqueueDeadGregoryPatches();
  std::shared_ptr<IEntity> contractEdge(const PointEntity &p1,
                                        const PointEntity &p2);

  void rebindReferences(const PointEntity &oldPoint, PointEntity &newPoint);
  void removeDeadEntities();
  void updateDirtyEntities();
};