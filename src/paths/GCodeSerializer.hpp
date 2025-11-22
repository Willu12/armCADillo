#pragma once

#include "millingPath.hpp"
#include <filesystem>

class GCodeSerializer {
public:
  static void serializePath(const MillingPath &millingPath,
                            const std::filesystem::path &filename);

private:
};