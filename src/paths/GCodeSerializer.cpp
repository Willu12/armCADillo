#include "GCodeSerializer.hpp"
#include "vec.hpp"
#include <fstream>
#include <ranges>

void GCodeSerializer::serializePath(const MillingPath &millingPath,
                                    const std::filesystem::path &filename) {

  std::ofstream out(filename);
  if (!out) {
    throw std::runtime_error("Failed to open file: " + filename.string());
  }

  for (const auto &[i, point] : millingPath.points() | std::views::enumerate) {
    out << std::format("N{}G01X{:.3f}Y{:.3f}Z{:.3f}\n", i, point.x() * 10.f,
                       point.z() * 10.f,
                       point.y() * 10.f); /// we reverse y and z here
  }
}