#pragma once

#include "graph.hpp"
#include "pointEntity.hpp"
#include <functional>
#include <map>
#include <vector>

struct Border {
  std::vector<std::reference_wrapper<const PointEntity>> points_;
  uint32_t uLen;
  uint32_t vLen;
};

class BorderGraph {
public:
  explicit BorderGraph(const std::vector<Border> &borders);
  std::vector<std::vector<std::reference_wrapper<const PointEntity>>>
  findHoles();

private:
  algebra::Graph _graph;
  std::unordered_map<std::size_t, std::reference_wrapper<const PointEntity>>
      _vertexPointMap;
  std::map<std::reference_wrapper<const PointEntity>, std::size_t>
      _pointVertexMap;
  void addBorder(const Border &border);
};