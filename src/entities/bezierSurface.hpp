#pragma once

#include "IEntity.hpp"
#include "IGroupedEntity.hpp"
#include "IMeshable.hpp"
#include "ISubscriber.hpp"
#include "bezierSurfaceMesh.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <memory>
#include <vector>

struct Patches {
  uint32_t sCount;
  uint32_t tCount;
};

class BezierSurface : public IGroupedEntity, public ISubscriber {
public:
  const MeshDensity &getMeshDensity() const { return _meshDensity; }
  MeshDensity &getMeshDensity() { return _meshDensity; }

  const Patches &getPatches() const { return _patches; }
  const IMeshable &getMesh() const override { return *_mesh; }
  const IMeshable &getPolyMesh() const { return *_polyMesh; }
  std::vector<std::shared_ptr<IEntity>> getPoints() {
    std::vector<std::shared_ptr<IEntity>> points;
    points.reserve(_points.size());
    for (const auto &point : _points) {
      points.emplace_back(point);
    }
    return points;
  }
  bool &wireframe() { return _wireframe; }
  const bool &wireframe() const { return _wireframe; }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurface(*this);
  };

  void update() override {
    _polyMesh = createPolyMesh();
    updateMesh();
  }
  void onSubscribableDestroyed(ISubscribable &publisher) override {}
  virtual uint32_t getColCount() = 0;
  virtual uint32_t getRowCount() = 0;
  std::vector<std::reference_wrapper<const PointEntity>>
  getPointsReferences() const override {
    std::vector<std::reference_wrapper<const PointEntity>> pointsReferences;
    pointsReferences.reserve(_points.size());
    for (const auto &point : _points) {
      pointsReferences.emplace_back(*point);
    }
    return pointsReferences;
  }

protected:
  std::vector<std::shared_ptr<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  std::unique_ptr<Mesh> _polyMesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
  Patches _patches = Patches{.sCount = 1, .tCount = 1};
  bool _wireframe = false;

  static std::vector<algebra::Vec3f>
  createCylinderPoints(const algebra::Vec3f &position, float r, float h) {
    const uint32_t u_patches = h / 0.5f;
    const uint32_t v_patches = 2;

    const uint32_t u_points = 3 * u_patches + 1;
    const uint32_t v_points = 3 * v_patches + 1;

    std::vector<algebra::Vec3f> controlPoints;
    controlPoints.reserve(u_points * v_points);

    for (uint32_t i = 0; i < u_points; ++i) {
      float u_ratio = float(i) / float(3 * u_patches);
      float angle = u_ratio * 2.0f * M_PI;

      float x_circle = std::cos(angle) * r;
      float y_circle = std::sin(angle) * r;

      for (uint32_t j = 0; j < v_points; ++j) {
        float v_ratio = float(j) / float(3 * v_patches);
        float z = v_ratio * h;

        controlPoints.emplace_back(x_circle + position[0],
                                   y_circle + position[1], z + position[2]);
      }
    }
    return controlPoints;
  }

  static std::vector<algebra::Vec3f>
  createFlatPoints(const algebra::Vec3f &position, uint32_t u_patches,
                   uint32_t v_patches) {
    std::vector<algebra::Vec3f> controlPoints;

    const float length = 0.3f;
    const uint32_t u_points = 3 * u_patches + 1;
    const uint32_t v_points = 3 * v_patches + 1;

    controlPoints.reserve(u_points * v_points);
    for (uint32_t i = 0; i < u_points; ++i) {
      for (uint32_t j = 0; j < v_points; ++j) {
        controlPoints.emplace_back(position[0] + float(i) / 3.f * length,
                                   position[1] + float(j) / 3.f * length,
                                   position[2]);
      }
    }
    return controlPoints;
  }

  std::unique_ptr<Mesh> createPolyMesh() {
    std::vector<float> vertices(3 * _points.size());

    for (int i = 0; i < _points.size(); ++i) {
      vertices[3 * i] = _points[i]->getPosition()[0];
      vertices[3 * i + 1] = _points[i]->getPosition()[1];
      vertices[3 * i + 2] = _points[i]->getPosition()[2];
    }

    auto rowCount = getRowCount();
    // 3 * _patches.sCount + 1;
    auto colCount = getColCount();
    //  3 * _patches.tCount + 1;
    std::vector<unsigned int> indices;

    // Horizontal lines (rows)
    for (int row = 0; row < rowCount; ++row) {
      for (int col = 0; col < colCount - 1; ++col) {
        int a = row * colCount + col;
        int b = a + 1;
        indices.push_back(a);
        indices.push_back(b); // horizontal line: a — b
      }
    }

    // Vertical lines (columns)
    for (int col = 0; col < colCount; ++col) {
      for (int row = 0; row < rowCount - 1; ++row) {
        int a = row * colCount + col;
        int b = a + colCount;
        indices.push_back(a);
        indices.push_back(b); // vertical line: a — b
      }
    }
    return Mesh::create(vertices, indices);
  }

  // Vertical lines (columns)
};