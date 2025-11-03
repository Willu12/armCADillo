#include "bezierSurface.hpp"

void BezierSurface::update() {
  _polyMesh = createPolyMesh();
  updateAlgebraicSurfaceC0();
  updateMesh();
}

std::vector<std::reference_wrapper<const PointEntity>>
BezierSurface::getPointsReferences() const {
  std::vector<std::reference_wrapper<const PointEntity>> points_references;
  points_references.reserve(_points.size());
  for (const auto &point : _points) {
    points_references.emplace_back(point);
  }

  return points_references;
}

std::unique_ptr<Mesh> BezierSurface::createPolyMesh() {
  std::vector<float> vertices(3 * _points.size());

  for (int i = 0; i < _points.size(); ++i) {
    vertices[3 * i] = _points[i].get().getPosition()[0];
    vertices[3 * i + 1] = _points[i].get().getPosition()[1];
    vertices[3 * i + 2] = _points[i].get().getPosition()[2];
  }

  auto row_count = getRowCount();
  auto col_count = getColCount();

  std::vector<unsigned int> indices;

  for (uint32_t row = 0; row < row_count; ++row) {
    for (uint32_t col = 0; col < col_count - 1; ++col) {
      auto a = row * col_count + col;
      auto b = a + 1;
      indices.push_back(a);
      indices.push_back(b);
    }
  }

  for (uint32_t col = 0; col < col_count; ++col) {
    for (uint32_t row = 0; row < row_count - 1; ++row) {
      auto a = row * col_count + col;
      auto b = a + col_count;
      indices.push_back(a);
      indices.push_back(b);
    }
  }
  return Mesh::create(vertices, indices);
}