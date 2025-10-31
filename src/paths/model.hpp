#pragma once

#include "bezierSurface.hpp"
#include <vector>
class Model {
public:
  explicit Model(std::vector<BezierSurface *> surfaces)
      : surfaces_(std::move(surfaces)) {}

  const std::vector<BezierSurface *> &surfaces() const { return surfaces_; }

private:
  /// here maybe we could use some smarter data structure
  std::vector<BezierSurface *> surfaces_;
};