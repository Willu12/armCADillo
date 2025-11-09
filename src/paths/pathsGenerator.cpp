#include "pathsGenerator.hpp"
#include "heightMap.hpp"
#include "model.hpp"
#include <memory>

void PathsGenerator::setModel(const std::vector<BezierSurface *> &surfaces) {
  model_ = std::make_unique<Model>(surfaces);
}

void PathsGenerator::run() {
  heightMap_ = std::make_unique<HeightMap>(
      heightMapGenerator_.generateHeightMap(*model_, block_));
  heightMap_->updateTexture();

  /*
/// roughing path
auto roughing_path = roughingPathGenerator_.generate(*heightMap_);
gCodeSerializer_.serializePath(roughing_path, "1.k16");
*/

  /// flat path
  Cutter cutter{
      .type_ = Cutter::Type::Flat,
      .diameter_ = 1.0f,
      .height_ = 1.0f,
  };

  flatPathGenerator_.setCutter(&cutter);
  flatPathGenerator_.setHeightMap(heightMap_.get());
  auto flat_path = flatPathGenerator_.generate();
  //  gCodeSerializer_.serializePath(flat_path, "2.f10");
}