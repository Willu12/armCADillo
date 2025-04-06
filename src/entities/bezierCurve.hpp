#pragma once
#include "IEntity.hpp"
#include "mesh.hpp"
#include "point.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class BezierCurve : public IEntity {
public:
  explicit BezierCurve(const std::vector<std::shared_ptr<Point>> points)
      : _points(points) {}

  void updateMesh() override {};

  const Mesh &getMesh() const override {}

private:
  std::vector<std::shared_ptr<Point>> _points;
  std::shared_ptr<Mesh> _mesh;
  //  uint32_t _segmentCount;

  std::vector<std::vector<std::shared_ptr<Point>>> createSegments() const {
    auto segments = std::vector<std::vector<std::shared_ptr<Point>>>();

    const uint32_t pointsPerSegment = 4;
    const uint32_t pointsFloored = (_points.size() / 4) * 4;
    uint32_t index = 0;

    while (index < pointsFloored) {
      std::vector<std::shared_ptr<Point>> segment;

      for (int j = 0; j < pointsPerSegment; ++j) {
        segment.push_back(_points[index++]);
      }
      index--;
    }
    return segments;
  }

  std::shared_ptr<Mesh> generateMesh() {
    // get segmetns and generate mesh from segments
    // auto segments = createSegments();
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