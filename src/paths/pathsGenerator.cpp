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

  // ------ Roughing Path ---------------------------
  Cutter roughing_cutter{
      .type_ = Cutter::Type::Ball,
      .diameter_ = 1.6f,
      .height_ = 2.f * 1.6f,
  };
  roughingPathGenerator_.setHeightMap(heightMap_.get());
  roughingPathGenerator_.setCutter(&roughing_cutter);
  auto roughing_path = roughingPathGenerator_.generate();
  gCodeSerializer_.serializePath(roughing_path, "1.k16");

  // ------ Flat Path ---------------------------
  Cutter flat_cutter{
      .type_ = Cutter::Type::Flat,
      .diameter_ = 1.0f,
      .height_ = 1.0f,
  };

  flatPathGenerator_.setCutter(&flat_cutter);
  flatPathGenerator_.setHeightMap(heightMap_.get());
  auto flat_path = flatPathGenerator_.generate();
  gCodeSerializer_.serializePath(flat_path, "2.f10");
}