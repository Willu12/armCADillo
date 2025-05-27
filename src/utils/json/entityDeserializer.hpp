#pragma once

#include "nlohmann/json.hpp"
#include "vec.hpp"

class IEntity;

class EntityDeserializer {
  using json = nlohmann::json;

public:
  virtual ~EntityDeserializer() = default;
  virtual std::shared_ptr<IEntity> deserializeEntity(const json &j) const = 0;

protected:
  algebra::Vec3f deserializePosition(const json &j) const;
  float deserializeScale(const json &j) const;
  algebra::Quaternion<float> deserializeRotation(const json &j) const;
};