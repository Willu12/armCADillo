#include "pathCombiner.hpp"
#include "GCodeSerializer.hpp"
#include "cutter.hpp"
#include "millingPath.hpp"
#include "namedPath.hpp"
#include "pathReader.hpp"
#include "vec.hpp"
#include <cstdint>
#include <memory>

void PathCombiner::addPaths(
    std::vector<std::filesystem::path> &millingPathFiles) {
  auto paths = MillingPathReader::readPaths(millingPathFiles);
  for (auto &path : paths) {
    millingPaths_.push_back(std::move(path));
  }
}

void PathCombiner::addPoint(const algebra::Vec3f &point) {
  std::string name("point" + std::to_string(millingPaths_.size()));
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
void PathCombiner::combinePaths(const std::vector<uint32_t> &pathIndices,
                                const std::string &name) {
  std::vector<algebra::Vec3f> points;
  for (const auto &index : pathIndices) {
    auto &path_points = millingPaths_[index]->points();
    points.insert(points.end(), path_points.begin(), path_points.end());
  }

  millingPaths_.emplace_back(std::make_unique<NamedPath>(points, name));
}

void PathCombiner::saveSelectedPath(uint32_t index) const {
  const auto &path = millingPaths_[index];
  Cutter cutter{.type_ = Cutter::Type::Ball, .diameter_ = 0.8f, .height_ = 1.6};

  MillingPath milling_path(path->points(), cutter);

  GCodeSerializer::serializePath(milling_path, path->name() + ".k08");
}

void PathCombiner::renamePath(uint32_t index, const std::string &name) {
  auto &path = millingPaths_[index];
  path->name() = name;
}

void PathCombiner::removePath(uint32_t pathIndex) {
  millingPaths_.erase(millingPaths_.begin() + pathIndex);
}

void PathCombiner::reducePath(uint32_t index) {
  if (index >= millingPaths_.size()) {
    return;
  }

  auto &path = millingPaths_[index];
  // Get reference to the vector so we can update it later
  auto &original_points = path->points();

  // If path is too small to reduce, do nothing
  if (original_points.size() < 3) {
    return;
  }

  std::vector<algebra::Vec3f> new_points;
  // Pre-allocate to prevent multiple reallocations
  new_points.reserve(original_points.size());

  // 1. Always keep the FIRST point
  new_points.push_back(original_points.front());

  // Define the threshold squared (0.1 * 0.1) to avoid expensive sqrt() calls
  const float min_dist_sq = 0.05f * 0.05f;

  // 2. Iterate through the middle points
  // We check distance against the 'new_points.back()' (the last VALID point
  // we added) rather than 'original_points[i-1]'. This filters out clusters
  // of tiny points.
  for (size_t i = 1; i < original_points.size() - 1; ++i) {
    const auto &current_point = original_points[i];
    const auto &last_accepted = new_points.back();

    // Calculate Squared Euclidean Distance
    algebra::Vec3f diff = current_point - last_accepted;
    float dist_sq = diff.dot(diff);

    // If the distance is significant enough, keep the point.
    // Otherwise, we skip it (it is effectively merged into the previous
    // line).
    if (dist_sq >= min_dist_sq) {
      new_points.push_back(current_point);
    }
  }

  // 3. Always keep the LAST point to preserve the end of the cut
  // Check to ensure we don't duplicate if the simplified path ended up very
  // close to the end
  const auto &last_original = original_points.back();
  const auto &last_new = new_points.back();

  float final_dist_sq =
      (last_original - last_new).dot(last_original - last_new);

  // Only add the last point if it's different from the current end,
  // OR if the result would be just 1 point (we need at least 2 for a line)
  if (final_dist_sq > 0.000001f || new_points.size() < 2) {
    new_points.push_back(last_original);
  }

  // 4. Update the path with the reduced list
  // Assuming 'points()' returns a non-const reference we can assign to
  std::string new_name = path->name() + "_reduced";
  millingPaths_.emplace_back(std::make_unique<NamedPath>(new_points, new_name));
}
