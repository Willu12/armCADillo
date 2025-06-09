#include "borderGraph.hpp"
#include "bezierSurfaceC0.hpp"
#include "graph.hpp"
#include "vec.hpp"
#include <functional>
#include <ranges>
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

  for (const auto &[key, value] : border.edgeInnerEdgeMap_) {
    _edgeInnerEdgeMap.insert_or_assign(key, value);
  }
}

std::vector<std::array<BorderEdge, 3>> BorderGraph::findHoles() const {
  const auto triangles = _graph.findAllTriangles();
  std::vector<std::array<BorderEdge, 3>> holes;
  holes.reserve(triangles.size());
  for (const auto &triangle : triangles) {
    holes.push_back(getEdge(triangle));
  }
  return holes;
}

std::array<BorderEdge, 3>
BorderGraph::getEdge(const std::array<std::size_t, 3> triangle) const {

  auto getEdgeFromTraingleEdge = [&](int u, int v) {
    return std::tie(_vertexPointMap.at(triangle[u]),
                    _vertexPointMap.at(triangle[v]));
  };
  Edge e1 = _edgeMap.contains(getEdgeFromTraingleEdge(0, 1))
                ? _edgeMap.at(getEdgeFromTraingleEdge(0, 1))
                : _edgeMap.at(getEdgeFromTraingleEdge(1, 0));
  Edge e2 = _edgeMap.contains(getEdgeFromTraingleEdge(1, 2))
                ? _edgeMap.at(getEdgeFromTraingleEdge(1, 2))
                : _edgeMap.at(getEdgeFromTraingleEdge(2, 1));
  Edge e3 = _edgeMap.contains(getEdgeFromTraingleEdge(0, 2))
                ? _edgeMap.at(getEdgeFromTraingleEdge(0, 2))
                : _edgeMap.at(getEdgeFromTraingleEdge(2, 0));

  return {BorderEdge{e1, _edgeInnerEdgeMap.at(e1)},
          BorderEdge{e2, _edgeInnerEdgeMap.at(e2)},
          BorderEdge{e3, _edgeInnerEdgeMap.at(e3)}};
}

Border::Border(const BezierSurfaceC0 &surface) {
  std::vector<std::reference_wrapper<const PointEntity>> borderPoints;
  const auto &points = surface.getPointsReferences();
  std::unordered_map<PointRefPair, Edge, RefPairHash, RefPairEqual>
      pointsEdgeMap;

  const uint32_t cols = surface.getColCount();
  const uint32_t rows = surface.getRowCount();

  // left border
  for (int row = 0; row < rows - 3; row += 3) {
    borderPoints.push_back(points[row]);

    auto edge =
        Edge{points[row], points[row + 1], points[row + 2], points[row + 3]};
    auto innerEdge = Edge{points[rows + row], points[rows + row + 1],
                          points[rows + row + 2], points[rows + row + 3]};
    pointsEdgeMap.insert({{points[row], points[row + 3]}, edge});
    edgeInnerEdgeMap_.insert({edge, innerEdge});
  }

  // Right border
  for (int row = 0; row < rows - 3; row += 3) {
    borderPoints.push_back(points[cols * (rows - 1) + rows - 1]);

    auto edge = Edge{points[cols * (rows - 1) + row],
                     points[cols * (rows - 1) + row + 1],
                     points[cols * (rows - 1) + row + 2],
                     points[cols * (rows - 1) + row + 3]};
    auto innerEdge = Edge{points[cols * (rows - 2) + row],
                          points[cols * (rows - 2) + row + 1],
                          points[cols * (rows - 2) + row + 2],
                          points[cols * (rows - 2) + row + 3]};
    pointsEdgeMap.insert(
        {{points[cols * (rows - 1) + row], points[cols * (rows - 1) + 3]},
         edge});

    edgeInnerEdgeMap_.insert({edge, innerEdge});
  }

  // bottom border
  for (int col = 0; col < cols - 3; col += 3) {
    borderPoints.push_back(points[(cols - 1) * rows]);

    auto edge = Edge{points[col * rows], points[(col + 1) * rows],
                     points[(col + 2) * rows], points[(col + 3) * rows]};
    auto innerEdge =
        Edge{points[col * rows + 1], points[(col + 1) * rows + 1],
             points[(col + 2) * rows + 1], points[(col + 3) * rows + 1]};
    pointsEdgeMap.insert(
        {{points[col * rows], points[(col + 3) * rows]}, edge});

    edgeInnerEdgeMap_.insert({edge, innerEdge});
  }

  // top border
  for (int col = 0; col < cols - 3; col += 3) {
    borderPoints.push_back(points[col * rows + rows - 1]);

    auto edge =
        Edge{points[col * rows + rows - 1], points[(col + 1) * rows + rows - 1],
             points[(col + 2) * rows + rows - 1],
             points[(col + 3) * rows + rows - 1]};
    auto innerEdge =
        Edge{points[col * rows + rows - 2], points[(col + 1) * rows + rows - 2],
             points[(col + 2) * rows + rows - 2],
             points[(col + 3) * rows + rows - 2]};
    pointsEdgeMap.insert(
        {{points[col * rows + rows - 1], points[(col + 3) * rows + rows - 1]},
         edge});
    edgeInnerEdgeMap_.insert({edge, innerEdge});
  }

  points_ = borderPoints;
  pointsEdgeMap_ = pointsEdgeMap;
  uLen = surface.getPatches().sCount;
  vLen = surface.getPatches().tCount;
};

std::array<algebra::Vec3f, 7> Edge::subdivide() const {
  std::array<algebra::Vec3f, 4> points;
  for (const auto &[i, p] : _points | std::views::enumerate)
    points[i] = p.get().getPosition();

  const auto &p0 = points[0];
  const auto &p1 = points[1];
  const auto &p2 = points[2];
  const auto &p3 = points[3];

  algebra::Vec3f p01 = (p0 + p1) * 0.5f;
  algebra::Vec3f p12 = (p1 + p2) * 0.5f;
  algebra::Vec3f p23 = (p2 + p3) * 0.5f;

  algebra::Vec3f p012 = (p01 + p12) * 0.5f;
  algebra::Vec3f p123 = (p12 + p23) * 0.5f;

  algebra::Vec3f p0123 = (p012 + p123) * 0.5f;

  return {p0, p01, p012, p0123, p123, p23, p3};
}