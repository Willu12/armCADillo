#include "entityDeserializer.hpp"
#include "nlohmann/json.hpp"
#include "pointEntity.hpp"
#include "scene.hpp"
#include "surface.hpp"
#include "vec.hpp"
#include <functional>

using json = nlohmann::json;

algebra::Vec3f EntityDeserializer::deserializePosition(const json &j) const {
  algebra::Vec3f pos;
  const auto &j_pos = j.at("position");
  j_pos.at("x").get_to(pos[0]);
  j_pos.at("y").get_to(pos[1]);
  j_pos.at("z").get_to(pos[2]);
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
  algebra::Quaternion<float> quaternion;
  const auto &j_quaternion = j.at("rotation");
  j_quaternion.at("x").get_to(quaternion.x());
  j_quaternion.at("y").get_to(quaternion.y());
  j_quaternion.at("z").get_to(quaternion.z());
  j_quaternion.at("w").get_to(quaternion.w());
  return quaternion;
}

std::vector<std::reference_wrapper<PointEntity>>
EntityDeserializer::getPoints(const json &j, const Scene &scene) const {
  const auto &control_points_json = j.at("controlPoints");
  const auto &points = scene.getPoints();
  std::vector<std::reference_wrapper<PointEntity>> points_references;

  for (const auto &point : control_points_json) {
    const auto &id = point.at("id");
    const auto iter = std::ranges::find_if(
        points, [id](const auto &p) { return p->getId() == id; });

    if (iter != points.end()) {
      auto *entity = dynamic_cast<PointEntity *>(*iter);
      points_references.emplace_back(*entity);
    }
  }
  return points_references;
}

algebra::ConnectionType EntityDeserializer::getConnectionType(
    const std::vector<std::reference_wrapper<PointEntity>> &points,
    size_t rowCount, size_t colCount, size_t degree) const {

  bool wrap_v = true;
  for (size_t row = 0; row < rowCount; ++row) {
    for (size_t k = 0; k < degree; ++k) {
      size_t left_index = row * colCount + k;
      size_t right_index = row * colCount + (colCount - degree + k);
      if (points[left_index].get().getId() !=
          points[right_index].get().getId()) {
        wrap_v = false;
        break;
      }
    }
    if (!wrap_v) {
      break;
    }
  }

  bool wrap_u = true;
  for (size_t col = 0; col < colCount; ++col) {
    for (size_t k = 0; k < degree; ++k) {
      size_t top_index = k * colCount + col;
      size_t bottom_index = (rowCount - degree + k) * colCount + col;
      if (points[top_index].get().getId() !=
          points[bottom_index].get().getId()) {
        wrap_u = false;
        break;
      }
    }
    if (!wrap_u) {
      break;
    }
  }

  if (wrap_u) {
    return algebra::ConnectionType::Rows;
  }
  if (wrap_v) {
    return algebra::ConnectionType::Columns;
  }
  return algebra::ConnectionType::Flat;
}