#pragma once

#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"

class RoughingPathGenerator {
public:
  MillingPath generate();
  void setHeightMap(const HeightMap *heightMap);
  void setCutter(const Cutter *cutter);

private:
  const HeightMap *heightMap_ = nullptr;
  const Cutter *cutter_ = nullptr;
  std::vector<algebra::Vec3f> calculateRoughMillingPoints() const;
};