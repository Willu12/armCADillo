#pragma once

#include "bezierSurfaceC2.hpp"
#include "entityDeserializer.hpp"
#include <memory>

class BezierSurfaceC2Deserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::shared_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    uint32_t uPatches{};
    uint32_t vPatches{};
    j.at("size").at("u").get_to(uPatches);
    j.at("size").at("v").get_to(vPatches);

    auto bezierSurfaceC2 =
        std::make_shared<BezierSurfaceC2>(points, uPatches, vPatches);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      bezierSurfaceC2->getName() = name;
    }
    j.at("samples").at("u").get_to(bezierSurfaceC2->getMeshDensity().s);
    j.at("samples").at("v").get_to(bezierSurfaceC2->getMeshDensity().t);

    return bezierSurfaceC2;
  }

private:
};