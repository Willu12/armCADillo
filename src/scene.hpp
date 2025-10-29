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
  explicit Scene(Camera *camera) : camera_(camera){};

  std::vector<IEntity *> getEntites() const;
  void addEntity(EntityType entityType, std::unique_ptr<IEntity> entity);
  Camera *getCamera();
  void removeEntities(std::vector<IEntity *> &entitiesToRemove);
  std::unordered_map<EntityType, std::vector<IEntity *>>
  getGroupedEntities() const;
  std::vector<IEntity *> getPickables() const;
  std::vector<IEntity *> getVirtualPoints() const;
  std::vector<IEntity *> getPoints() const;

  void processFrame();
  friend class GUI;

private:
  Camera *camera_;
  std::unordered_map<EntityType, std::vector<std::unique_ptr<IEntity>>>
      entities_;
  std::vector<IEntity *> deadEntities_;

  void filterEntitiesToRemove(std::vector<IEntity *> &entitiesToRemove);

  void enqueueSurfacePoints(std::vector<IEntity *> &entitiesToRemove) const;
  void enqueueDeadGregoryPatches();
  IEntity *contractEdge(const PointEntity &p1, const PointEntity &p2);

  void rebindReferences(const PointEntity &oldPoint, PointEntity &newPoint);
  void removeDeadEntities();
  void updateDirtyEntities();
};