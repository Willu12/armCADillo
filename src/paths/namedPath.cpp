#include "namedPath.hpp"
#include <ranges>

void NamedPath::createMesh() {
  std::vector<float> vertices(3 * points_.size());
  std::vector<uint32_t> indices;

  for (const auto &[i, p] : points_ | std::views::enumerate) {
    p.y() = p.y();
    for (int j = 0; j < 3; ++j) {
      vertices[i * 3 + j] = p[j];
    }
  }

  auto count = static_cast<int32_t>(points_.size());
  auto range1 = std::views::iota(0, count - 1);
  auto range2 = std::views::iota(1, count);

  for (auto [a, b] : std::views::zip(range1, range2)) {
    indices.push_back(a);
    indices.push_back(b);
  }

  mesh_ = Mesh::create(vertices, indices);
}