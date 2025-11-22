#pragma once

#include "namedPath.hpp"
#include "vec.hpp"
#include <filesystem>
#include <memory>
#include <vector>

class MillingPathReader {
public:
  static std::vector<std::unique_ptr<NamedPath>>
  readPaths(std::vector<std::filesystem::path> &millingPathFiles);

  static std::unique_ptr<NamedPath>
  readPath(const std::filesystem::path &millingPathFile);

private:
  static algebra::Vec3f readLine(const std::string &line);
  static float parseCoordinate(const std::string &coordinate);
};