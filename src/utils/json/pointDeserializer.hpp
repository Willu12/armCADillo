#pragma once

#include "entityDeserializer.hpp"
#include "pointEntity.hpp"
#include <memory>

class PointDeserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::shared_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    auto pos = deserializePosition(j);

    auto point = PointEntity(pos);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      point.getName() = name;
    }
    auto p = std::make_shared<PointEntity>(point);
    p->setId(id);
    return p;
  }

private:
};