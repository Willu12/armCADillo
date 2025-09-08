#pragma once

#include "entitiesTypes.hpp"
#include "entityBuilders/IEntityBuilder.hpp"
#include "entityBuilders/bezierCurveC0Builder.hpp"
#include "entityBuilders/bezierCurveC2Builder.hpp"
#include "entityBuilders/bezierSufaceC2Builder.hpp"
#include "entityBuilders/bezierSurfaceC0Builder.hpp"
#include "entityBuilders/gregoryPatchBuilder.hpp"
#include "entityBuilders/interpolatingSplineBuilder.hpp"
#include "entityBuilders/pointBuilder.hpp"
#include "entityBuilders/polylineBuilder.hpp"
#include "entityBuilders/torusBuilder.hpp"
#include "entityFactory.hpp"
#include <map>
#include <memory>
#include <string>

class GUI;

class EntityUtils {
public:
  explicit EntityUtils(GUI *gui, EntityFactory *entityFactory)
      : gui_(gui), entityFactory_(entityFactory) {
    registerBuilders();
  }

  const std::map<std::string, EntityType> &getStringEntityMap() const {
    return creatableEntitiesMap_;
  }

  const std::map<EntityType, std::unique_ptr<IEntityBuilder>> &
  getEntityBuilders() const {
    return entityBuilders_;
  }

private:
  void registerBuilders() {
    entityBuilders_.emplace(EntityType::Torus, std::make_unique<TorusBuilder>(
                                                   gui_, entityFactory_));
    entityBuilders_.emplace(EntityType::Point, std::make_unique<PointBuilder>(
                                                   gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::BezierCurveC0,
        std::make_unique<BezierCurveC0Builder>(gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::BSplineCurve,
        std::make_unique<BSplineCurveBuilder>(gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::InterpolatingSplineCurve,
        std::make_unique<InterpolatingSplineBuilder>(gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::BezierSurfaceC0,
        std::make_unique<BezierSurfaceC0Builder>(gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::BezierSurfaceC2,
        std::make_unique<BezierSurfaceC2Builder>(gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::GregorySurface,
        std::make_unique<GregoryPatchBuilder>(gui_, entityFactory_));
    entityBuilders_.emplace(
        EntityType::Polyline,
        std::make_unique<PolylineBuilder>(gui_, entityFactory_));
  }

  std::map<std::string, EntityType> creatableEntitiesMap_{
      {"Torus", EntityType::Torus},
      {"Point", EntityType::Point},
      {"bezierCurveC0", EntityType::BezierCurveC0},
      {"BSplineCurve", EntityType::BSplineCurve},
      {"InterpolatingSpline", EntityType::InterpolatingSplineCurve},
      {"BezierSurfaceC0", EntityType::BezierSurfaceC0},
      {"BezierSurfaceC2", EntityType::BezierSurfaceC2},
      {"GregorySurface", EntityType::GregorySurface},
      {"Polyline", EntityType::Polyline},
  };

  GUI *gui_;
  EntityFactory *entityFactory_;
  std::map<EntityType, std::unique_ptr<IEntityBuilder>> entityBuilders_;
};
