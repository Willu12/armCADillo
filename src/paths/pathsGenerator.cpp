#include "pathsGenerator.hpp"
#include "cutter.hpp"
#include "millingPath.hpp"
#include "vec.hpp"

MillingPath PathsGenerator::roughingPath() {
  /// change later
  Cutter cutter{
      .type_ = Cutter::Type::Ball,
      .diameter_ = 1.6f,
      .height_ = 2.f * 1.6f,
  };

  auto height_map = heightMapGenerator_.generateHeightMap(model_, block_);
  auto milling_points = calculateRoughMillingPoints(height_map, cutter);
  return MillingPath(std::move(milling_points), cutter);
};

std::vector<algebra::Vec3f>
PathsGenerator::calculateRoughMillingPoints(const HeightMap &heightMap,
                                            const Cutter &cutter) const {
  std::vector<algebra::Vec3f> milling_points;
  auto starting_point = algebra::Vec3f(-10.f, 5.f + cutter.height_, 10.f);

  return milling_points;
}