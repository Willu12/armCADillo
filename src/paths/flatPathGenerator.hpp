#pragma once

#include "color.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "millingPath.hpp"
#include "vec.hpp"
#include <cstdint>
#include <limits>
#include <list>
#include <unordered_map>
#include <vector>

static constexpr uint32_t kMaxIndex = std::numeric_limits<uint32_t>::max();

class FlatPathGenerator {
public:
  struct Segment {

    algebra::Vec2f start_;
    algebra::Vec2f end_;

    uint32_t startContourIndex_{};
    uint32_t endContourIndex_ = kMaxIndex;
  };

  MillingPath generate();
  void setCutter(const Cutter *cutter);
  void setHeightMap(HeightMap *heightMap);

private:
  const Cutter *cutter_ = nullptr;
  HeightMap *heightMap_ = nullptr;
  std::unordered_map<uint32_t, algebra::Vec3f> boundaryNormalMap_;
  std::vector<algebra::Vec3f> contourPoints_;

  std::vector<uint32_t> findBoundaryIndices();
  std::vector<algebra::Vec3f> findCutterPositionsFromBoundary(
      const std::vector<uint32_t> &boundaryIndices) const;

  std::vector<std::list<FlatPathGenerator::Segment>> generateSegments() const;

  void removeSelfIntersections();

  std::vector<std::vector<algebra::Vec3f>> generatePaths(
      std::vector<std::list<FlatPathGenerator::Segment>> &segments) const;

  const std::vector<algebra::Vec3f> &generateContourPath() const;

  MillingPath combineLocalPaths(
      const std::vector<std::vector<algebra::Vec3f>> &localPaths) const;

  /// helperFunction
  void paintBorderRed(const std::vector<uint32_t> &boundaryIndices) const;
  void paintBorder(const std::vector<algebra::Vec3f> &contour,
                   Color color) const;
};