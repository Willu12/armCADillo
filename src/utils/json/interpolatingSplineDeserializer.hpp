#pragma once

#include "entityDeserializer.hpp"
#include "interpolatingSplineC2.hpp"
#include <memory>

class InterpolatingSplineDeserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::shared_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    auto interpolatingSpline = std::make_shared<InterpolatingSplineC2>(points);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      interpolatingSpline->getName() = name;
    }
    interpolatingSpline->getId() = id;
    return interpolatingSpline;
  }

private:
};