#pragma once
#include "IEntity.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class BezierCurve : public IEntity {
public:
  explicit BezierCurve(
      const std::vector<std::reference_wrapper<PointEntity>> points)
      : _points(points), _mesh(generateMesh()) {}

  void updateMesh() override {};

  const Mesh &getMesh() const override { return *_mesh; }

private:
  std::vector<std::reference_wrapper<PointEntity>> _points;
  std::unique_ptr<Mesh> _mesh;
  //  uint32_t _segmentCount;

  std::vector<std::vector<std::reference_wrapper<PointEntity>>>
  createSegments() const {
    auto segments =
        std::vector<std::vector<std::reference_wrapper<PointEntity>>>();

    const uint32_t pointsPerSegment = 4;
    const uint32_t pointsFloored = (_points.size() / 4) * 4;
    uint32_t index = 0;

    while (index < pointsFloored) {
      std::vector<std::reference_wrapper<PointEntity>> segment;

      for (int j = 0; j < pointsPerSegment; ++j) {
        segment.push_back(_points[index++]);
      }
      index--;
    }
    return segments;
  }

  std::unique_ptr<Mesh> generateMesh() {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i < _points.size(); ++i) {
      auto worldPosition = _points[i].get().getPosition();
      vertices.push_back(worldPosition[0]);
      vertices.push_back(worldPosition[1]);
      vertices.push_back(worldPosition[2]);
    }
    for (uint32_t i = 0; i + 3 < _points.size(); i += 3) {
      indices.push_back(i);
      indices.push_back(i + 1);
      indices.push_back(i + 2);
      indices.push_back(i + 3);
    }

    return Mesh::create(vertices, indices);
  }
};