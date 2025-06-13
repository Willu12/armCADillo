#pragma once

#include "bezierSurfaceC0.hpp"
#include "graph.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

using PointRefPair = std::pair<std::reference_wrapper<PointEntity>,
                               std::reference_wrapper<PointEntity>>;

struct RefPairHash {
  std::size_t operator()(const PointRefPair &p) const {
    auto h1 = std::hash<PointEntity *>{}(&p.first.get());
    auto h2 = std::hash<PointEntity *>{}(&p.second.get());
    return h1 ^ (h2 << 1);
  }
};

struct PointHash {
  std::size_t operator()(const std::reference_wrapper<PointEntity> &p) const {
    return std::hash<PointEntity *>{}(&p.get());
  }
};

struct PointEqual {
  bool operator()(const std::reference_wrapper<PointEntity> &p1,
                  const std::reference_wrapper<PointEntity> &p2) const {
    return &p1.get() == &p2.get();
  }
};

struct RefPairEqual {
  bool operator()(const PointRefPair &a, const PointRefPair &b) const {
    return &a.first.get() == &b.first.get() &&
           &a.second.get() == &b.second.get();
  }
};

struct Edge {
  std::array<std::reference_wrapper<PointEntity>, 4> _points;

  std::array<algebra::Vec3f, 7> subdivide() const;
};

struct EdgeEqual {
  bool operator()(const Edge &a, const Edge &b) const {
    for (int i = 0; i < 4; ++i) {
      if (a._points[i].get().getId() != b._points[i].get().getId())
        return false;
    }
    return true;
  }
};

struct EdgeHash {
  std::size_t operator()(const Edge &e) const {
    std::size_t seed = 0;
    for (int i = 0; i < 4; ++i) {
      auto id = e._points[i].get().getId();
      seed ^= std::hash<int>{}(id) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

struct BorderEdge {
  Edge _edge;
  Edge _innerEdge;
};

struct Border {
  std::vector<std::reference_wrapper<PointEntity>> points_;
  std::unordered_map<PointRefPair, Edge, RefPairHash, RefPairEqual>
      pointsEdgeMap_;
  std::unordered_map<Edge, Edge, EdgeHash, EdgeEqual> edgeInnerEdgeMap_;
  uint32_t uLen;
  uint32_t vLen;

  Border(BezierSurfaceC0 &surface);
};

class BorderGraph {
public:
  explicit BorderGraph(const std::vector<Border> &borders);
  std::vector<std::array<BorderEdge, 3>> findHoles() const;

private:
  algebra::Graph _graph;
  std::unordered_map<std::size_t, std::reference_wrapper<PointEntity>>
      _vertexPointMap;
  std::unordered_map<std::reference_wrapper<PointEntity>, std::size_t,
                     PointHash, PointEqual>
      _pointVertexMap;
  std::unordered_map<PointRefPair, Edge, RefPairHash, RefPairEqual> _edgeMap;
  std::unordered_map<Edge, Edge, EdgeHash, EdgeEqual> _edgeInnerEdgeMap;
  void addBorder(const Border &border);
  std::array<BorderEdge, 3>
  getEdge(const std::array<std::size_t, 3> triangle) const;
};