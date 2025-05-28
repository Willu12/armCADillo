#pragma once

#include "nlohmann/json.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <utility>

class IEntity;
class Scene;

class EntityDeserializer {
  using json = nlohmann::json;

public:
  virtual ~EntityDeserializer() = default;
  virtual std::shared_ptr<IEntity> deserializeEntity(const json &j,
                                                     Scene &scene) const = 0;

protected:
  algebra::Vec3f deserializePosition(const json &j) const;
  algebra::Vec3f deserializeScale(const json &j) const;
  algebra::Quaternion<float> deserializeRotation(const json &j) const;

  std::vector<std::reference_wrapper<PointEntity>>
  getPoints(const json &j, const Scene &scene) const;
};