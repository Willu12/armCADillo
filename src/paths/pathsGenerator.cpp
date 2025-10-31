#include "pathsGenerator.hpp"
#include "cutter.hpp"

MillingPath PathsGenerator::roughingPath() {
  /// change later
  Cutter cutter{
      .type_ = Cutter::Type::Ball,
      .diameter_ = 16,
      .height_ = 2 * 16,
  };

  heightMapGenerator_.generateHeightMap(model_);
};