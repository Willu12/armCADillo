#pragma once

#include "bezierSurfaceC0.hpp"
#include "entityDeserializer.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <print>

class BezierSurfaceC0Deserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::unique_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);
    uint32_t row_count{};
    uint32_t col_count{};
    j.at("size").at("v").get_to(col_count);
    j.at("size").at("u").get_to(row_count);
    auto connection_type =
        EntityDeserializer::getConnectionType(points, row_count, col_count, 1);
    auto bezier_surface_c0 = std::make_unique<BezierSurfaceC0>(
        points, (row_count - 1) / 3, (col_count - 1) / 3, connection_type);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      bezier_surface_c0->getName() = name;
    }
    j.at("samples").at("u").get_to(bezier_surface_c0->getMeshDensity().s);
    j.at("samples").at("v").get_to(bezier_surface_c0->getMeshDensity().t);
    bezier_surface_c0->getId() = id;

    return bezier_surface_c0;
  }

private:
};