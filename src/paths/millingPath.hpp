#pragma once

#include "cutter.hpp"
#include "vec.hpp"
#include <vector>

class MillingPath {
public:
  MillingPath(std::vector<algebra::Vec3f> points, Cutter cutter)
      : points_(std::move(points)), cutter_(cutter) {}

  const Cutter &cutter() const { return cutter_; }
  const std::vector<algebra::Vec3f> &points() const { return points_; }

private:
  std::vector<algebra::Vec3f> points_;
  Cutter cutter_;
};