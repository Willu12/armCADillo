#include "entityDeserializer.hpp"
#include "IEntity.hpp"
#include "vec.hpp"

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