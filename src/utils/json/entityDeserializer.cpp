#include "entityDeserializer.hpp"
#include "IEntity.hpp"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "vec.hpp"
#include <functional>
#include <memory>

using json = nlohmann::json;

algebra::Vec3f EntityDeserializer::deserializePosition(const json &j) const {
  algebra::Vec3f pos;
  const auto &jPos = j.at("position");
  jPos.at("x").get_to(pos[0]);
  jPos.at("y").get_to(pos[1]);
  jPos.at("z").get_to(pos[2]);
  return pos;
}

float EntityDeserializer::deserializeScale(const json &j) const {
  float scale = 0.f;
  j.at("scale").at("x").get_to(scale);
  return scale;
}

algebra::Quaternion<float>
EntityDeserializer::deserializeRotation(const json &j) const {
  algebra::Quaternion<float> Q;
  const auto &jQ = j.at("rotation");
  jQ.at("x").get_to(Q.x());
  jQ.at("y").get_to(Q.y());
  jQ.at("z").get_to(Q.z());
  jQ.at("w").get_to(Q.w());
  return Q;
}

std::vector<std::reference_wrapper<PointEntity>>
EntityDeserializer::getPoints(const json &j, const Scene &scene) const {
  const auto &controlPointsJson = j.at("controlPoints");
  const auto &points = scene.getPoints();
  std::vector<std::reference_wrapper<PointEntity>> pointsRef;

  for (const auto &point : controlPointsJson) {
    const auto &Id = point.at("id");
    const auto iter = std::ranges::find_if(
        points, [Id](const auto &p) { return p->getId() == Id; });

    if (iter != points.end()) {
      pointsRef.emplace_back(*std::dynamic_pointer_cast<PointEntity>(*iter));
    }
  }
  return pointsRef;
}