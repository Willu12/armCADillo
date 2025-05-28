#include "bezierCurveC0.hpp"
#include "bezierCurveC0Deserializer.hpp"

std::shared_ptr<IEntity>
BezierCurveC0Deserializer::deserializeEntity(const json &j,
                                             Scene &scene) const {
  std::string name;
  int id = -1;
  j.at("id").get_to(id);

  const auto points = getPoints(j, scene);

  auto bezierCurveC0 = std::make_shared<BezierCurveC0>(points);
  if (j.contains("name")) {
    j.at("name").get_to(name);
    bezierCurveC0->getName() = name;
  }
  bezierCurveC0->getId() = id;
  return bezierCurveC0;
}
