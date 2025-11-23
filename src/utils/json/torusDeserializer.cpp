#include "torusDeserializer.hpp"
#include "nlohmann/json.hpp"
#include "torusEntity.hpp"
std::unique_ptr<IEntity>
TorusDeserializer::deserializeEntity(const json &j, Scene & /*scene*/) const {
  std::string name;
  int id = -1;
  MeshDensity mesh_density;
  float small_radius = 0.f;
  float big_radius = 0.f;
  j.at("id").get_to(id);
  j.at("samples").at("u").get_to(mesh_density.s);
  j.at("samples").at("v").get_to(mesh_density.t);
  j.at("smallRadius").get_to(small_radius);
  j.at("largeRadius").get_to(big_radius);

  auto pos = deserializePosition(j);
  auto scale = deserializeScale(j);
  auto rotation = deserializeRotation(j);

  auto torus = std::make_unique<TorusEntity>(big_radius, small_radius, pos,
                                             mesh_density);
  torus->getRotation() = rotation.normalized();
  torus->getScale() = scale;

  if (j.contains("name")) {
    j.at("name").get_to(name);
    torus->getName() = name;
  }

  torus->getId() = id;
  return torus;
}