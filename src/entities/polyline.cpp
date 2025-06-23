#include "polyline.hpp"
#include "mesh.hpp"
#include "vec.hpp"
#include <ranges>
#include <vector>

Polyline::Polyline(std::vector<algebra::Vec3f> &points) : _points(points) {
  _id = kClassId++;
  _name = "Polyline_" + std::to_string(_id);
  _mesh = generateMesh();
}
void Polyline::addPoint(algebra::Vec3f &point) {
  _points.emplace_back(point);
  updateMesh();
}

const std::vector<algebra::Vec3f> &Polyline::getPoints() const {
  return _points;
}

void Polyline::updateMesh() { _mesh = generateMesh(); }

const IMeshable &Polyline::getMesh() const { return *_mesh; }

std::unique_ptr<Mesh> Polyline::generateMesh() {
  std::vector<float> vertices(3 * _points.size());
  std::vector<uint32_t> indices;

  for (const auto &[i, p] : _points | std::views::enumerate) {
    for (int j = 0; j < 3; ++j) {
      vertices[i * 3 + j] = p[j];
    }
  }

  auto count = static_cast<int32_t>(_points.size());
  auto range1 = std::views::iota(0, count - 1);
  auto range2 = std::views::iota(1, count);

  for (auto [a, b] : std::views::zip(range1, range2)) {
    indices.push_back(a);
    indices.push_back(b);
  }

  return Mesh::create(vertices, indices);
}
