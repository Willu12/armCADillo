#pragma once
#include "IRenderable.hpp"
#include "pointEntity.hpp"
#include <vector>

class PolygonalCurve : public IRenderable {
public:
  PolygonalCurve(std::vector<PointEntity *> points)
      : _points(points), _mesh(generateMesh()) {}

  algebra::Mat4f getModelMatrix() const override {
    return algebra::Mat4f::Identity();
  }
  const Mesh &getMesh() const override { return *_mesh; }
  const algebra::Vec3f &getPosition() const override { return _position; }
  algebra::Vec3f &getPosition() override { return _position; }
  void updateMesh() { _mesh = generateMesh(); }

private:
  std::vector<PointEntity *> _points;
  std::shared_ptr<Mesh> _mesh;
  algebra::Vec3f _position;

  std::shared_ptr<Mesh> generateMesh() {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i < _points.size(); ++i) {
      auto worldPosition = _points[i]->getPosition();
      vertices.push_back(worldPosition[0]);
      vertices.push_back(worldPosition[1]);
      vertices.push_back(worldPosition[2]);

      if (i > 0) {
        indices.push_back(i - 1);
        indices.push_back(i);
      }
    }
    return Mesh::create(vertices, indices);
  }
};