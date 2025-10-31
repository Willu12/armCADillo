#include "GCodeSerializer.hpp"
#include "vec.hpp"
#include <fstream>
#include <ranges>

void GCodeSerializer::serializePath(
    const MillingPath &millingPath,
    const std::filesystem::path &filename) const {

  std::ofstream out(filename);
  if (!out) {
    throw std::runtime_error("Failed to open file: " + filename.string());
  }

  for (const auto &[i, point] : millingPath.points() | std::views::enumerate) {
    out << std::format("N{}G01X{:.3f}Y{:.3f}Z{:.3f}", i, point.x(), point.y(),
                       point.z());
  }
}