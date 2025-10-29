#pragma once

#include "entityDeserializer.hpp"
#include "interpolatingSplineC2.hpp"
#include <memory>

#include "nlohmann/json.hpp"

class InterpolatingSplineDeserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::unique_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    auto interpolating_spline = std::make_unique<InterpolatingSplineC2>(points);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      interpolating_spline->getName() = name;
    }
    interpolating_spline->getId() = id;
    return interpolating_spline;
  }

private:
};