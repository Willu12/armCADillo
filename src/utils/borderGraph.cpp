#include "borderGraph.hpp"
#include "graph.hpp"
#include <vector>
BorderGraph::BorderGraph(const std::vector<Border> &borders) {
  for (const auto &border : borders)
    addBorder(border);
}

void BorderGraph::addBorder(const Border &border) {

  for (const auto &point : border.points_) {
    if (!_pointVertexMap.contains(point)) {
      auto v = _graph.addVertex();
      _pointVertexMap.insert({point, v});
      _vertexPointMap.insert({v, point});
    }
  }

  for (uint32_t u = 0; u < border.uLen - 1; ++u) {
    for (uint32_t v = 0; v < border.vLen - 1; ++v) {
      const auto &currPoint = border.points_[u * border.uLen + v];
      const auto &rightPoint = border.points_[u * border.uLen + v + 1];
      const auto &downPoint = border.points_[(u + 1) * border.uLen + v];

      _graph.addEdge(_pointVertexMap.at(currPoint),
                     _pointVertexMap.at(rightPoint));
      _graph.addEdge(_pointVertexMap.at(currPoint),
                     _pointVertexMap.at(downPoint));
    }
  }
}

std::vector<std::vector<std::reference_wrapper<const PointEntity>>>
BorderGraph::findHoles() {
  const auto triangles = _graph.findAllTriangles();
  std::vector<std::vector<std::reference_wrapper<const PointEntity>>> holes;
  holes.reserve(triangles.size());
  for (const auto &triangle : triangles) {
    holes.push_back({_vertexPointMap[triangle[0]], _vertexPointMap[triangle[1]],
                     _vertexPointMap[triangle[2]]});
  }
  return holes;
}