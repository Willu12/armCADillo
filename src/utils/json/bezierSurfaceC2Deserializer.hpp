#pragma once

#include "bezierSurfaceC2.hpp"
#include "entityDeserializer.hpp"
#include "surface.hpp"
#include <memory>
#include <ostream>
#include <print>

class BezierSurfaceC2Deserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::shared_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    uint32_t uPoints{};
    uint32_t vPoints{};
    j.at("size").at("u").get_to(uPoints);
    j.at("size").at("v").get_to(vPoints);

    auto connectionType =
        EntityDeserializer::getConnectionType(points, vPoints, uPoints, 3);

    auto bezierSurfaceC2 = std::make_shared<BezierSurfaceC2>(
        points, uPoints - 3, vPoints - 3, connectionType);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      bezierSurfaceC2->getName() = name;
    }
    j.at("samples").at("u").get_to(bezierSurfaceC2->getMeshDensity().s);
    j.at("samples").at("v").get_to(bezierSurfaceC2->getMeshDensity().t);
    bezierSurfaceC2->getId() = id;
    return bezierSurfaceC2;
  }

private:
};