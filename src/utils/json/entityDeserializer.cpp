#include "entityDeserializer.hpp"
#include "IEntity.hpp"
#include "bezierSurface.hpp"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <functional>
#include <memory>

using json = nlohmann::json;

algebra::Vec3f EntityDeserializer::deserializePosition(const json &j) const {
  algebra::Vec3f pos;
  const auto &jPos = j.at("position");
  jPos.at("x").get_to(pos[0]);
  jPos.at("y").get_to(pos[1]);
  jPos.at("z").get_to(pos[2]);
  return pos;
}

algebra::Vec3f EntityDeserializer::deserializeScale(const json &j) const {
  algebra::Vec3f scale;
  j.at("scale").at("x").get_to(scale[0]);
  j.at("scale").at("y").get_to(scale[1]);
  j.at("scale").at("z").get_to(scale[2]);
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

std::vector<std::reference_wrapper<PointEntity>>
EntityDeserializer::getPoints(const json &j, const Scene &scene) const {
  const auto &controlPointsJson = j.at("controlPoints");
  const auto &points = scene.getPoints();
  std::vector<std::reference_wrapper<PointEntity>> pointsRef;

  for (const auto &point : controlPointsJson) {
    const auto &Id = point.at("id");
    const auto iter = std::ranges::find_if(
        points, [Id](const auto &p) { return p->getId() == Id; });

    if (iter != points.end()) {
      pointsRef.emplace_back(*std::dynamic_pointer_cast<PointEntity>(*iter));
    }
  }
  return pointsRef;
}

algebra::ConnectionType EntityDeserializer::getConnectionType(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    size_t rowCount, size_t colCount, size_t degree) const {

  bool wrapV = true;
  for (size_t row = 0; row < rowCount; ++row) {
    for (size_t k = 0; k < degree; ++k) {
      size_t leftIndex = row * colCount + k;
      size_t rightIndex = row * colCount + (colCount - degree + k);
      if (points[leftIndex].get().getId() != points[rightIndex].get().getId()) {
        wrapV = false;
        break;
      }
    }
    if (!wrapV) {
      break;
    }
  }

  bool wrapU = true;
  for (size_t col = 0; col < colCount; ++col) {
    for (size_t k = 0; k < degree; ++k) {
      size_t topIndex = k * colCount + col;
      size_t bottomIndex = (rowCount - degree + k) * colCount + col;
      if (points[topIndex].get().getId() != points[bottomIndex].get().getId()) {
        wrapU = false;
        break;
      }
    }
    if (!wrapU) {
      break;
    }
  }

  if (wrapU) {
    return algebra::ConnectionType::Rows;
  }
  if (wrapV) {
    return algebra::ConnectionType::Columns;
  }
  return algebra::ConnectionType::Flat;
}