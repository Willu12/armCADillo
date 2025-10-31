#pragma once

#include "cutter.hpp"
#include "vec.hpp"
#include <vector>

class MillingPath {
public:
  const Cutter &cutter() const { return cutter_; }
  const std::vector<algebra::Vec3f> &points() const { return points_; }

private:
  std::vector<algebra::Vec3f> points_;
  Cutter cutter_;
};