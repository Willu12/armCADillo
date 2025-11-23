#include "bezierCurveC0.hpp"
#include "bezierCurveC0Deserializer.hpp"
#include "nlohmann/json.hpp"
#include <memory>

std::unique_ptr<IEntity>
BezierCurveC0Deserializer::deserializeEntity(const json &j,
                                             Scene &scene) const {
  std::string name;
  int id = -1;
  j.at("id").get_to(id);

  const auto points = getPoints(j, scene);

  auto bezier_curve_c0 = std::make_unique<BezierCurveC0>(points);
  if (j.contains("name")) {
    j.at("name").get_to(name);
    bezier_curve_c0->getName() = name;
  }
  bezier_curve_c0->getId() = id;
  return bezier_curve_c0;
}
