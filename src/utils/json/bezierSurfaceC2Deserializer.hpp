#pragma once

#include "bezierSurfaceC2.hpp"
#include "entityDeserializer.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <print>

class BezierSurfaceC2Deserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::unique_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    uint32_t u_points{};
    uint32_t v_points{};
    j.at("size").at("u").get_to(u_points);
    j.at("size").at("v").get_to(v_points);

    auto connection_type =
        EntityDeserializer::getConnectionType(points, v_points, u_points, 3);

    auto bezier_surface_c2 = std::make_unique<BezierSurfaceC2>(
        points, u_points - 3, v_points - 3, connection_type);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      bezier_surface_c2->getName() = name;
    }
    j.at("samples").at("u").get_to(bezier_surface_c2->getMeshDensity().s);
    j.at("samples").at("v").get_to(bezier_surface_c2->getMeshDensity().t);
    bezier_surface_c2->getId() = id;
    return bezier_surface_c2;
  }

private:
};