#pragma once

#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "segment.hpp"
#include "vec.hpp"
#include <cstdint>
#include <list>
#include <unordered_map>
#include <vector>

class FlatPathGenerator {
public:
  MillingPath generate() const;
  void setCutter(const Cutter *cutter);
  void setHeightMap(HeightMap *heightMap);

private:
  const Cutter *cutter_ = nullptr;
  HeightMap *heightMap_ = nullptr;

  std::unordered_map<uint32_t, algebra::Vec3f> createBoundaryNormalMap() const;
  std::vector<uint32_t> findBoundaryIndices() const;
  std::vector<algebra::Vec3f> findCutterPositionsFromBoundary(
      const std::vector<uint32_t> &boundaryIndices,
      const std::unordered_map<uint32_t, algebra::Vec3f> &boundaryNormalMap)
      const;

  std::vector<std::list<Segment>>
  generateSegments(const std::vector<algebra::Vec3f> &countourPoints) const;

  std::vector<algebra::Vec3f>
  removeSelfIntersections(const std::vector<algebra::Vec3f> &points) const;

  std::vector<std::vector<algebra::Vec3f>>
  generatePaths(std::vector<std::list<Segment>> &segments) const;

  /// helperFunction
  void paintBorderRed(const std::vector<uint32_t> &boundaryIndices) const;
  void paintBorderRed(const std::vector<algebra::Vec3f> &contour) const;
};