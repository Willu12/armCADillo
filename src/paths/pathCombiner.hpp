#pragma once

#include "GCodeSerializer.hpp"
#include "millingPath.hpp"
#include "namedPath.hpp"
#include "vec.hpp"
#include <filesystem>

class PathCombiner {
public:
  void addPaths(std::vector<std::filesystem::path> &millingPathFiles);
  void addPoint(const algebra::Vec3f &point);

  MillingPath createCombinedPath(const std::vector<uint32_t> &pathIndices);
  void removePath(uint32_t pathIndex);

  const std::vector<std::unique_ptr<NamedPath>> &millingPaths() const {
    return millingPaths_;
  }

private:
  std::vector<std::unique_ptr<NamedPath>> millingPaths_;
};