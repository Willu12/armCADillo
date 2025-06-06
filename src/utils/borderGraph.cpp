#include "borderGraph.hpp"
#include "graph.hpp"
#include <functional>
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

  for (const auto &pair : border.pointsEdgeMap_) {
    const Edge &edge = pair.second;
    _graph.addEdge(_pointVertexMap.at(edge._points[0]),
                   _pointVertexMap.at(edge._points[3]));
  }

  for (const auto &[key, value] : border.pointsEdgeMap_) {
    _edgeMap.insert_or_assign(key, value);
  }
}

std::vector<std::array<std::reference_wrapper<const Edge>, 3>>
BorderGraph::findHoles() const {
  const auto triangles = _graph.findAllTriangles();
  std::vector<std::array<std::reference_wrapper<const Edge>, 3>> holes;
  holes.reserve(triangles.size());
  for (const auto &triangle : triangles) {
    holes.push_back(getEdge(triangle));
  }
  return holes;
}

std::array<std::reference_wrapper<const Edge>, 3>
BorderGraph::getEdge(const std::array<std::size_t, 3> triangle) const {

  Edge e1 = _edgeMap.at(std::tie(_vertexPointMap.at(triangle[0]),
                                 _vertexPointMap.at(triangle[1])));
  Edge e2 = _edgeMap.at(std::tie(_vertexPointMap.at(triangle[1]),
                                 _vertexPointMap.at(triangle[2])));
  Edge e3 = _edgeMap.at(std::tie(_vertexPointMap.at(triangle[2]),
                                 _vertexPointMap.at(triangle[0])));

  return {e1, e2, e3};
}