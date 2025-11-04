#pragma once

#include "millingPath.hpp"
#include <filesystem>

class GCodeSerializer {
public:
  void serializePath(const MillingPath &millingPath,
                     const std::filesystem::path &filename) const;

private:
};