#pragma once

#include "IEntity.hpp"
#include "IGroupedEntity.hpp"
#include "ISubscriber.hpp"
#include "vec.hpp"
#include <functional>

class Polyline : public IEntity {
public:
  void addPoint(algebra::Vec3f &point);
  void updateMesh() override;
  const std::vector<algebra::Vec3f> &getPoints() const;
  explicit Polyline(std::vector<algebra::Vec3f> &points);
  const IMeshable &getMesh() const override;

private:
  std::vector<algebra::Vec3f> _points;
  std::unique_ptr<Mesh> _mesh;
  inline static int kClassId = 0;

  std::unique_ptr<Mesh> generateMesh();
};