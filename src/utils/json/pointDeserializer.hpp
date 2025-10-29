#pragma once

#include "entityDeserializer.hpp"
#include "pointEntity.hpp"
#include <memory>

class PointDeserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::unique_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    auto pos = deserializePosition(j);

    auto point = std::make_unique<PointEntity>(pos);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      point->getName() = name;
    }
    // auto p = std::make_unique<PointEntity>(std::move(point));
    point->setId(id);
    return std::move(point);
  }

private:
};