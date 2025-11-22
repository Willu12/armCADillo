#pragma once

#include "mesh.hpp"
#include "vec.hpp"
#include <memory>
#include <string>
#include <vector>
class NamedPath {
public:
  NamedPath(std::vector<algebra::Vec3f> points, std::string name)
      : points_(std::move(points)), name_(std::move(name)) {
    createMesh();
  }

  const Mesh &mesh() const { return *mesh_; }
  const std::string &name() const { return name_; }

  const std::vector<algebra::Vec3f> &points() const { return points_; }
  std::vector<algebra::Vec3f> &points() { return points_; }

private:
  std::vector<algebra::Vec3f> points_;
  std::string name_;
  std::unique_ptr<Mesh> mesh_;

  void createMesh();
};