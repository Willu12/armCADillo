#include "pathReader.hpp"
#include "namedPath.hpp"
#include "vec.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

std::vector<std::unique_ptr<NamedPath>> MillingPathReader::readPaths(
    std::vector<std::filesystem::path> &millingPathFiles) {
  std::ranges::sort(millingPathFiles);

  std::vector<std::unique_ptr<NamedPath>> milling_path_queue;

  milling_path_queue.reserve(millingPathFiles.size());

  for (const auto &filepath : millingPathFiles) {
    milling_path_queue.emplace_back(readPath(filepath));
  }

  return milling_path_queue;
}

std::unique_ptr<NamedPath>
MillingPathReader::readPath(const std::filesystem::path &millingPathFile) {
  std::vector<algebra::Vec3f> points;

  std::ifstream file(millingPathFile);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + millingPathFile.string());
  }

  for (const auto &line : std::views::istream<std::string>(file)) {
    try {
      algebra::Vec3f point = readLine(line);
      points.push_back(point);
    } catch (const std::exception &e) {
      throw std::runtime_error("Warning: Skipping invalid line " + line + " " +
                               e.what());
    }
  }

  return std::make_unique<NamedPath>(std::move(points),
                                     millingPathFile.filename());
}
algebra::Vec3f MillingPathReader::readLine(const std::string &line) {
  size_t x_pos = line.find('X');
  size_t y_pos = line.find('Y');
  size_t z_pos = line.find('Z');

  if (x_pos == std::string::npos || y_pos == std::string::npos ||
      z_pos == std::string::npos) {
    throw std::invalid_argument("Line missing X, Y, or Z coordinate");
  }

  std::string x_str = line.substr(x_pos + 1, y_pos - x_pos - 1);
  std::string y_str = line.substr(y_pos + 1, z_pos - y_pos - 1);
  std::string z_str = line.substr(z_pos + 1, line.size() - z_pos);

  float x = parseCoordinate(x_str);
  float y = parseCoordinate(y_str);
  float z = parseCoordinate(z_str);

  return algebra::Vec3f(x, z, y);
}
float MillingPathReader::parseCoordinate(const std::string &coordinate) {

  float sign = coordinate[0] == '-' ? -1.f : 1.f;
  size_t offset = sign == 1.0f ? 0 : 1;

  size_t dot_position = coordinate.find('.');

  std::string integer_substr = coordinate.substr(offset, dot_position - offset);

  int integer_part = 0;
  int decimal_factor = 1;
  for (int index = static_cast<int>(integer_substr.size()) - 1; index >= 0;
       index--) {
    integer_part += (integer_substr[index] - ('0')) * decimal_factor;
    decimal_factor *= 10;
  }

  int fractional_part = (coordinate[dot_position + 1] - '0') * 100 +
                        (coordinate[dot_position + 2] - '0') * 10 +
                        (coordinate[dot_position + 3] - '0');
  float result = static_cast<float>(integer_part) +
                 static_cast<float>(fractional_part) / 1000.0f;

  return sign * result / 10.f;
}
