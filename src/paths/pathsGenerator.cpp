#include "pathsGenerator.hpp"
#include "model.hpp"
#include "roughingPathGenerator.hpp"

void PathsGenerator::setModel(const std::vector<BezierSurface *> &surfaces) {
  model_ = std::make_unique<Model>(surfaces);
}

void PathsGenerator::run() {
  auto height_map = heightMapGenerator_.generateHeightMap(*model_, block_);

  /// roughing path
  auto roughing_path = roughingPathGenerator_.generate(height_map);
  gCodeSerializer_.serializePath(roughing_path, "1.k16");

  /// flat path
  auto flat_path = flatPathGenerator_.generate(height_map);
  gCodeSerializer_.serializePath(flat_path, "2.f10");
}