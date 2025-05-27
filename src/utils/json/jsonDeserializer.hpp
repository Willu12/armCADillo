#pragma once
#include "entitiesTypes.hpp"
#include "entityDeserializer.hpp"
#include "nlohmann/json.hpp"
#include "pointDeserializer.hpp"
#include "scene.hpp"
#include "torusDeserializer.hpp"
#include <fstream>
#include <memory>
#include <unordered_map>

class JsonDeserializer {
  using json = nlohmann::json;

public:
  JsonDeserializer() {
    initTypeMap();
    initDeserializerMap();
  }

  void loadScence(const std::string &path, Scene &scene) const {
    auto json = readJson(path);
    auto geometryJson = json.at("geometry");
    auto pointJson = json.at("points");

    for (const auto &json : pointJson) {
      const auto &deserializer = _deserializerMap.at(EntityType::Point);
      const auto &entity = deserializer->deserializeEntity(json);
      scene.addEntity(EntityType::Point, entity);
    }

    for (const auto &json : geometryJson) {
      const auto entityType = _typeMap.at(json.at("objectType"));
      const auto &deserializer = _deserializerMap.at(entityType);
      const auto &entity = deserializer->deserializeEntity(json);
      scene.addEntity(entityType, entity);
    }
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
    _deserializerMap.insert(
        {EntityType::Point, std::make_unique<PointDeserializer>()});
  }

  json readJson(const std::string &path) const {
    std::ifstream i(path);
    json j;
    i >> j;
    return j;
  }
};
