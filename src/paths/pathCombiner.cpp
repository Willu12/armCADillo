#include "pathCombiner.hpp"
#include "GCodeSerializer.hpp"
#include "cutter.hpp"
#include "millingPath.hpp"
#include "namedPath.hpp"
#include "pathReader.hpp"
#include "vec.hpp"
#include <memory>

void PathCombiner::addPaths(
    std::vector<std::filesystem::path> &millingPathFiles) {
  auto paths = MillingPathReader::readPaths(millingPathFiles);
  for (auto &path : paths) {
    millingPaths_.push_back(std::move(path));
  }
}

void PathCombiner::addPoint(const algebra::Vec3f &point) {
  std::string name(&"point"[millingPaths_.size()]);
  std::vector points{point};
  auto path = std::make_unique<NamedPath>(points, name);
  millingPaths_.push_back(std::move(path));
}
MillingPath
PathCombiner::createCombinedPath(const std::vector<uint32_t> &pathIndices) {
  std::vector<algebra::Vec3f> points;
  for (const auto &index : pathIndices) {
    auto &path_points = millingPaths_[index]->points();
    points.insert(points.end(), path_points.begin(), path_points.end());
  }
  Cutter cutter{.type_ = Cutter::Type::Ball, .diameter_ = 0.8f, .height_ = 1.6};

  MillingPath milling_path(std::move(points), cutter);

  GCodeSerializer::serializePath(milling_path, "3.k08");

  return milling_path;
}

void PathCombiner::removePath(uint32_t pathIndex) {
  millingPaths_.erase(millingPaths_.begin() + pathIndex);
}