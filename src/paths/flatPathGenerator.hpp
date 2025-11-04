#pragma once

#include "heightMap.hpp"
#include "millingPath.hpp"

class FlatPathGenerator {
public:
  MillingPath generate(const HeightMap &heightMap) const;

private:
};