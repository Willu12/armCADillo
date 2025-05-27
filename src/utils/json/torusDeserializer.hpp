#pragma once

#include "entityDeserializer.hpp"
#include <memory>
class TorusDeserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::shared_ptr<IEntity> deserializeEntity(const json &j) const final;

private:
};