#pragma once
#include "entitiesTypes.hpp"
#include "entityDeserializer.hpp"
#include "nlohmann/json.hpp"
#include "torusDeserializer.hpp"
#include "torusEntity.hpp"
#include <memory>
#include <unordered_map>

class jsonDeserializer {
public:
  jsonDeserializer() {
    initTypeMap();
    initDeserializerMap();
  }

private:
  std::unordered_map<std::string, EntityType> _typeMap;
  std::unordered_map<EntityType, std::unique_ptr<EntityDeserializer>>
      _deserializerMap;

  void initTypeMap() {
    _typeMap = {{"torus", EntityType::Torus},
                {"point", EntityType::Point},
                {"bezierC2", EntityType::BSplineCurve},
                {"interpolatedC2", EntityType::InterpolatingSplineCurve},
                {"bezierC0", EntityType::BezierCurveC0},
                {"bezierSurfaceC0", EntityType::BezierSurfaceC0},
                {"bezierSurfaceC2", EntityType::BezierSurfaceC2}};
  }
  void initDeserializerMap() {
    _deserializerMap.insert(
        {EntityType::Torus, std::make_unique<TorusDeserializer>()});
  }
};
