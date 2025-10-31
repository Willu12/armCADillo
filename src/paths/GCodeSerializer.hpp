#pragma once

#include "millingPath.hpp"
#include <filesystem>

class GCodeSerializer {
  void serializePath(const MillingPath &millingPath,
                     const std::filesystem::path &filename) const;
};