#pragma once

#include "entityDeserializer.hpp"
#include <memory>

class BezierCurveC0Deserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::unique_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final;

private:
};