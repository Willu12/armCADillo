#pragma once

#include "IEntity.hpp"
#include "mesh.hpp"
#include "vec.hpp"

class Polyline : public IEntity {
public:
  explicit Polyline(const std::vector<algebra::Vec3f> &points);

  void addPoint(algebra::Vec3f &point);
  void updateMesh() override;
  const std::vector<algebra::Vec3f> &getPoints() const;
  const IMeshable &getMesh() const override;
  std::vector<algebra::Vec3f> getSparsePoints(float delta) const;

private:
  std::vector<algebra::Vec3f> points_;
  std::unique_ptr<Mesh> mesh_;
  inline static int kClassId = 0;

  std::unique_ptr<Mesh> generateMesh();
};