#include "torusDeserializer.hpp"
#include "torusEntity.hpp"
std::shared_ptr<IEntity>
TorusDeserializer::deserializeEntity(const json &j, Scene &scene) const {
  std::string name;
  int id = -1;
  MeshDensity meshDensity;
  float smallRadius = 0.f;
  float bigRadius = 0.f;
  j.at("id").get_to(id);
  j.at("samples").at("u").get_to(meshDensity.s);
  j.at("samples").at("v").get_to(meshDensity.t);
  j.at("smallRadius").get_to(smallRadius);
  j.at("largeRadius").get_to(bigRadius);

  auto pos = deserializePosition(j);
  auto scale = deserializeScale(j);
  auto rotation = deserializeRotation(j);

  auto torus = TorusEntity(bigRadius, smallRadius, pos);
  torus.getRotation() = rotation;
  torus.getScale() = scale;
  if (j.contains("name")) {
    j.at("name").get_to(name);
    torus.getName() = name;
  }

  return std::make_shared<TorusEntity>(torus);
}