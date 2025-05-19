#pragma once

#include "IEntity.hpp"
#include "ISubscriber.hpp"
#include "bezierSurfaceMesh.hpp"
#include "mesh.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <vector>

struct Patches {
  uint32_t sCount;
  uint32_t tCount;
};

class BezierSurface : public IEntity, public ISubscriber {
public:
  const MeshDensity &getMeshDensity() const { return _meshDensity; }
  MeshDensity &getMeshDensity() { return _meshDensity; }

  const Patches &getPatches() const { return _patches; }
  const IMeshable &getMesh() const override { return *_mesh; }
  std::vector<std::shared_ptr<IEntity>> getPoints() {
    std::vector<std::shared_ptr<IEntity>> points;
    points.reserve(_points.size());
    for (const auto &point : _points) {
      points.emplace_back(point);
    }
    return points;
  }
  const bool &wireframe() const { return _wireframe; }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitBezierSurface(*this);
  };

  void update() override { updateMesh(); }
  void onSubscribableDestroyed(ISubscribable &publisher) override {}

protected:
  std::vector<std::shared_ptr<PointEntity>> _points;
  std::unique_ptr<BezierSurfaceMesh> _mesh;
  MeshDensity _meshDensity = MeshDensity{.s = 4, .t = 4};
  Patches _patches = Patches{.sCount = 1, .tCount = 1};
  bool _wireframe = true;

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
};