#pragma once

#include "bezierSurfaceC0.hpp"
#include "entityDeserializer.hpp"
#include "pointEntity.hpp"
#include <functional>
#include <memory>

class BezierSurfaceC0Deserializer : public EntityDeserializer {
  using json = nlohmann::json;

public:
  std::shared_ptr<IEntity> deserializeEntity(const json &j,
                                             Scene &scene) const final {
    std::string name;
    int id = -1;
    j.at("id").get_to(id);

    const auto points = getPoints(j, scene);

    uint32_t rowCount{};
    uint32_t colCount{};
    j.at("size").at("v").get_to(colCount);
    j.at("size").at("u").get_to(rowCount);

    auto bezierSurfaceC0 = std::make_shared<BezierSurfaceC0>(
        points, (rowCount - 1) / 3, (colCount - 1) / 3);
    if (j.contains("name")) {
      j.at("name").get_to(name);
      bezierSurfaceC0->getName() = name;
    }
    j.at("samples").at("u").get_to(bezierSurfaceC0->getMeshDensity().s);
    j.at("samples").at("v").get_to(bezierSurfaceC0->getMeshDensity().t);
    bezierSurfaceC0->getId() = id;
    return bezierSurfaceC0;
  }

private:
};