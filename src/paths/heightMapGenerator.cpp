#include "heightMapGenerator.hpp"
#include "bezierSurface.hpp"
#include "bezierSurfaceC0.hpp"
#include "block.hpp"
#include "heightMap.hpp"
#include "vec.hpp"
#include <fstream>
#include <sstream>

static constexpr float kBaseHeight = 1.5f;
static constexpr uint32_t kDivisions = 4000;
static constexpr uint32_t kBaseDivisions = 1500;

HeightMap HeightMapGenerator::generateHeightMap(const Model &model,
                                                const Block &block) {
  HeightMap height_map(Divisions{.x_ = kBaseDivisions, .z_ = kBaseDivisions},
                       kBaseHeight, &block);

  for (const auto *surface : model.surfaces()) {
    processSurface(*surface, height_map);
  }

  height_map.saveToFile();
  // generateFromFiles("../../resources/maps/height_map.txt",
  //                   "../../resources/maps/normal_map.txt", height_map);
  return height_map;
}

void HeightMapGenerator::processSurface(const BezierSurface &surface,
                                        HeightMap &heightMap) const {

  const uint32_t u_divisions = kDivisions;
  const uint32_t v_divisions = kDivisions;

  for (uint32_t u_index = 0; u_index < u_divisions; ++u_index) {
    for (uint32_t v_index = 0; v_index < v_divisions; ++v_index) {
      float u = static_cast<float>(u_index) / static_cast<float>(u_divisions);
      float v = static_cast<float>(v_index) / static_cast<float>(v_divisions);

      auto surface_point = surface.value({u, v});

      //  if (surface_point.y() < 0.f) {
      //    continue;
      //  }

      auto height_map_index = heightMap.posToIndex(surface_point);
      if (surface_point.y() + kBaseHeight > heightMap.at(height_map_index)) {
        heightMap.at(height_map_index) =
            std::max(kBaseHeight, surface_point.y() + kBaseHeight);
        heightMap.normalAtIndex(height_map_index) = surface.normal({u, v});
      }
    }
  }
}

void HeightMapGenerator::generateFromFiles(const std::string &height_file,
                                           const std::string &normal_file,
                                           HeightMap &heightMap) const {
  {
    std::ifstream in(height_file);
    if (!in) {
      throw std::runtime_error("Failed to open " + height_file +
                               " for reading.");
    }

    std::vector<float> heights;
    std::string line;

    while (std::getline(in, line)) {
      std::istringstream iss(line);
      float h{};
      while (iss >> h) {
        heights.push_back(h);
      }
    }

    if (heights.empty()) {
      throw std::runtime_error("Height map file is empty or invalid: " +
                               height_file);
    }

    heightMap.data_ = std::move(heights);
  }

  // --- Load normal map ---
  {
    std::ifstream in(normal_file);
    if (!in) {
      throw std::runtime_error("Failed to open " + normal_file +
                               " for reading.");
    }

    std::vector<algebra::Vec3f> normals;
    std::string line;

    while (std::getline(in, line)) {
      std::istringstream iss(line);
      float nx = NAN;
      float ny = NAN;
      float nz = NAN;
      while (iss >> nx >> ny >> nz) {
        normals.emplace_back(nx, ny, nz);
      }
    }

    if (normals.empty()) {
      throw std::runtime_error("Normal map file is empty or invalid: " +
                               normal_file);
    }

    heightMap.normalData_ = std::move(normals);
  }

  if (heightMap.data_.size() != heightMap.normalData_.size()) {
    throw std::runtime_error("Height and normal maps have mismatched sizes.");
  }
}