#pragma once

#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "segment.hpp"
#include "vec.hpp"
#include <cstdint>
#include <list>
#include <vector>

class FlatPathGenerator {
public:
  MillingPath generate() const;
  void setCutter(const Cutter *cutter);
  void setHeightMap(HeightMap *heightMap);

private:
  const Cutter *cutter_ = nullptr;
  HeightMap *heightMap_ = nullptr;

  std::vector<uint32_t> findBoundaryIndices() const;
  std::vector<algebra::Vec3f> findCutterPositionsFromBoundary(
      const std::vector<uint32_t> &boundaryIndices) const;

  std::vector<std::list<Segment>>
  generateSegments(const std::vector<algebra::Vec3f> &countourPoints) const;

  void
  removeSelfIntersections(std::vector<algebra::Vec3f> &contourPoints) const;

  std::vector<std::vector<algebra::Vec3f>>
  generatePaths(std::vector<std::list<Segment>> &segments) const;

  /// helperFunction
  void paintBorderRed(const std::vector<uint32_t> &boundaryIndices) const;
};