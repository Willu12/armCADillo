#include "polyline.hpp"
#include "mesh.hpp"
#include "vec.hpp"
#include <ranges>
#include <vector>

Polyline::Polyline(const std::vector<algebra::Vec3f> &points)
    : points_(points) {
  _id = kClassId++;
  _name = "Polyline_" + std::to_string(_id);
  mesh_ = generateMesh();
}
void Polyline::addPoint(algebra::Vec3f &point) {
  points_.emplace_back(point);
  updateMesh();
}

const std::vector<algebra::Vec3f> &Polyline::getPoints() const {
  return points_;
}

void Polyline::updateMesh() { mesh_ = generateMesh(); }

const IMeshable &Polyline::getMesh() const { return *mesh_; }

std::unique_ptr<Mesh> Polyline::generateMesh() {
  std::vector<float> vertices(3 * points_.size());
  std::vector<uint32_t> indices;

  for (const auto &[i, p] : points_ | std::views::enumerate) {
    for (int j = 0; j < 3; ++j) {
      vertices[i * 3 + j] = p[j];
    }
  }

  auto count = static_cast<int32_t>(points_.size());
  auto range1 = std::views::iota(0, count - 1);
  auto range2 = std::views::iota(1, count);

  for (auto [a, b] : std::views::zip(range1, range2)) {
    indices.push_back(a);
    indices.push_back(b);
  }

  return Mesh::create(vertices, indices);
}

std::vector<algebra::Vec3f> Polyline::getSparsePoints(float delta) const {
  std::vector<algebra::Vec3f> points;
  points.push_back(points_.front());
  for (const auto &point : points_) {
    if ((points.back() - point).length() > delta) {
      points.push_back(point);
    }
  }
  return points;
}
