#pragma once

#include "EntityFactories/IEntityFactory.hpp"
#include "EntityFactories/pointFactory.hpp"
#include "IEntity.hpp"
#include "bezierCurveC0.hpp"
#include "entitiesTypes.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <unordered_map>
class EntityFactory {
  EntityFactory() { initEntityFactories(); }
  /*
  std::shared_ptr<IEntity> createPoint(const algebra::Vec3f &position) {
    return std::make_shared<PointEntity>(position);
  }
  std::shared_ptr<IEntity> createTorus(const algebra::Vec3f &position) {
    return std::make_shared<PointEntity>(1.f, .3f, position);
  }
  std::shared_ptr<IEntity> createBezierCurve(
      const std::vector<std::reference_wrapper<PointEntity>> &points) {
    return std::make_shared<BezierCurveC0>(points);
  }
  std::shared_ptr<IEntity> createBSplineCurve();
  std::shared_ptr<IEntity> createInterpolatingSplineCurve();
  std::shared_ptr<IEntity> createBezierSurfaceC0();
  */
  std::shared_ptr<IEntity> create(EntityType entityType,
                                  const algebra::Vec3f &position) {
    return _entityFactories.at(entityType)->create(position);
  }

private:
  std::unordered_map<EntityType, std::unique_ptr<IEntityFactory>>
      _entityFactories;

  void initEntityFactories() {
    _entityFactories = {
        {EntityType::Point, std::make_unique<PointFactory>()},
        {EntityType::Torus, std::make_unique<PointFactory>()},
    };
  }
};