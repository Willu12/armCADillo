#pragma once

#include "bSplineCurve.hpp"
#include "entityDeserializer.hpp"
#include <memory>

class BSplineDeserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::unique_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    auto bSplineCurve = std::make_unique<BSplineCurve>(points);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      bSplineCurve->getName() = name;
    }
    bSplineCurve->getId() = id;
    return bSplineCurve;
  }

private:
};