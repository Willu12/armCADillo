#pragma once

#include <algorithm>
#include <array>
#include <numeric>
#include <stack>
#include <stdexcept>
#include <tuple>
#include <vector>
namespace algebra {
using Vertex = std::size_t;
using Edge = std::tuple<Vertex, Vertex>;

class Graph {
public:
  Graph() : algebra::Graph(0){};
  Graph(const Graph &G) = default;
  explicit Graph(Vertex size);
  explicit Graph(const std::vector<std::tuple<Vertex, Vertex>> &edges);
  Vertex addVertex();
  void addEdge(Vertex u, Vertex v);
  void addEdges(std::vector<std::tuple<Vertex, Vertex>> edges);
  bool removeVertex(Vertex v);
  bool removeVertices(const std::vector<Vertex> &verticesSortedDesc);
  bool removeVertices(const std::vector<Vertex> &verticesSortedDesc,
                      const int *toBeRemoved, const Vertex *vertexIndexDelta);
  bool removeEdge(Vertex u, Vertex v);
  bool hasEdge(Vertex u, Vertex v) const;
  bool connected() const;
  std::size_t degree(Vertex v) const;
  std::size_t size() const;
  std::size_t edgeCount() const;
  std::vector<std::tuple<Vertex, Vertex>> arcs() const;
  std::vector<std::tuple<Vertex, Vertex>> edges() const;
  std::vector<std::array<Vertex, 3>> findAllTriangles() const;
  std::vector<Vertex> neighbours(Vertex v) const;

  void addGraph(const Graph &G);

private:
  std::vector<std::vector<Vertex>> _adjacencyList;
};
inline Graph::Graph(Vertex size) {
  Graph::_adjacencyList = std::vector<std::vector<Vertex>>(size);
}

inline Graph::Graph(const std::vector<std::tuple<Vertex, Vertex>> &edges) {
  Vertex max_value = 0;
  for (auto [u, v] : edges) {
    max_value = std::max({max_value, u, v});
  }

  Vertex size = max_value + 1;
  _adjacencyList = std::vector<std::vector<Vertex>>(size);

  this->addEdges(edges);
}

inline std::size_t Graph::size() const { return this->_adjacencyList.size(); }

inline Vertex Graph::addVertex() {
  this->_adjacencyList.emplace_back();
  return this->_adjacencyList.size() - 1;
}

inline void Graph::addEdge(Vertex u, Vertex v) {
  if (u > v)
    std::swap(v, u);
  if (std::ranges::find(this->_adjacencyList[u], v) !=
      this->_adjacencyList[u].end())
    return; // throw std::runtime_error("adding existing edge\n");
  Graph::_adjacencyList[u].push_back(v);
}

inline bool Graph::hasEdge(Vertex u, Vertex v) const {
  if (u > v)
    std::swap(v, u);
  auto u_neighbours = neighbours(u);
  return std::ranges::find(u_neighbours, v) != u_neighbours.end();
}

inline void Graph::addEdges(std::vector<std::tuple<Vertex, Vertex>> edges) {
  for (auto [u, v] : edges) {
    this->addEdge(u, v);
  }
}

inline std::vector<std::tuple<Vertex, Vertex>> Graph::arcs() const {
  auto edges = std::vector<std::tuple<Vertex, Vertex>>();

  for (Vertex i = 0; i < this->size(); i++) {
    for (auto neighbour : this->neighbours(i)) {
      edges.emplace_back(std::tie(i, neighbour));
    }
  }
  return edges;
}

inline std::vector<std::tuple<Vertex, Vertex>> Graph::edges() const {
  auto edges = std::vector<std::tuple<Vertex, Vertex>>();

  for (Vertex i = 0; i < this->size(); i++) {
    for (auto neighbour : _adjacencyList[i]) {
      edges.emplace_back(std::tie(i, neighbour));
    }
  }
  return edges;
}

inline bool Graph::removeVertex(Vertex v) {
  if (v > 0 && this->size() < (v - 1))
    return false;

  // find Vertex remove all edges, push back all edges from upcoming vertices
  for (Vertex i = 0; i < this->size(); i++) {
    if (i == v)
      continue;
    if (std::ranges::find(this->_adjacencyList[i], v) !=
        this->_adjacencyList[i].end())
      this->removeEdge(i, v);
    auto neighbours = this->_adjacencyList[i];
    for (Vertex neighbour_index = 0; neighbour_index < neighbours.size();
         neighbour_index++) {
      auto neighbour = this->_adjacencyList[i][neighbour_index];
      if (neighbour > v)
        this->_adjacencyList[i][neighbour_index]--;
    }
  }

  for (Vertex i = v; i < this->size() - 1; i++) {
    this->_adjacencyList[i] = this->_adjacencyList[i + 1];
  }
  this->_adjacencyList.pop_back();

  return true;
}

inline bool
Graph::removeVertices(const std::vector<Vertex> &verticesSortedDesc) {
  // 1. Preprocessing
  int *toBeRemoved = new int[_adjacencyList.size()]{false};
  for (auto v : verticesSortedDesc)
    toBeRemoved[v] = true;
  Vertex *VertexIndexDelta = new Vertex[_adjacencyList.size() + 1];
  VertexIndexDelta[0] = 0;
  std::partial_sum(toBeRemoved, toBeRemoved + _adjacencyList.size(),
                   VertexIndexDelta + 1);

  removeVertices(verticesSortedDesc, toBeRemoved, VertexIndexDelta);

  // 4. Cleanup
  delete[] toBeRemoved;
  delete[] VertexIndexDelta;

  return true;
}

inline bool Graph::removeVertices(const std::vector<Vertex> &verticesSortedDesc,
                                  const int *toBeRemoved,
                                  const Vertex *VertexIndexDelta) {
  // 2. Remove incoming edges
  for (auto &neighbours : _adjacencyList) {
    std::erase_if(neighbours, [toBeRemoved](auto v) { return toBeRemoved[v]; });
    std::transform(
        neighbours.begin(), neighbours.end(), neighbours.begin(),
        [VertexIndexDelta](auto v) { return v - VertexIndexDelta[v]; });
  }

  // 3. Remove outgoing edges
  for (auto v : verticesSortedDesc) {
    _adjacencyList.erase(_adjacencyList.begin() + v);
  }

  return true;
}

inline bool Graph::removeEdge(Vertex u, Vertex v) {
  if (u > v) {
    std::swap(u, v);
  }
  auto v_iterator = std::find(this->_adjacencyList[u].begin(),
                              this->_adjacencyList[u].end(), v);
  if (v_iterator == this->_adjacencyList[u].end()) {
    return false;
  }

  this->_adjacencyList[u].erase(v_iterator);
  return true;
}

inline std::vector<Vertex> Graph::neighbours(Vertex v) const {
  std::vector<Vertex> neighbours;

  for (Vertex neigh = 0; neigh < v; neigh++) {
    const auto &neigh_out = _adjacencyList[neigh];
    if (std::ranges::find(neigh_out, v) != neigh_out.end()) {
      neighbours.emplace_back(neigh);
    }
  }
  for (auto neigh : _adjacencyList[v]) {
    neighbours.emplace_back(neigh);
  }

  return neighbours;
}

inline Vertex Graph::edgeCount() const {
  Vertex edgeCount = 0;

  for (Vertex i = 0; i < this->size(); i++) {
    edgeCount += this->_adjacencyList[i].capacity();
  }
  return edgeCount;
}

inline std::size_t Graph::degree(Vertex v) const {
  return neighbours(v).size();
}

inline bool Graph::connected() const {
  if (this->size() == 0) {
    return true;
  }

  std::vector<bool> visited(this->size(), false);
  std::stack<Vertex> stack;
  stack.push(0);
  visited[0] = true;

  while (!stack.empty()) {
    std::size_t current = stack.top();
    stack.pop();
    for (auto neighbor : this->neighbours(current)) {
      if (!visited[neighbor]) {
        visited[neighbor] = true;
        stack.push(neighbor);
      }
    }
  }

  for (std::size_t i = 0; i < this->size(); ++i) {
    if (!visited[i]) {
      return false;
    }
  }
  return true;
}

inline void Graph::addGraph(const Graph &G) {
  auto edge_offset = this->size();
  for (Vertex v = 0; v < G.size(); ++v) {
    addVertex();
  }

  for (const auto &[u, v] : G.edges()) {
    addEdge(u + edge_offset, v + edge_offset);
  }
}
inline std::vector<std::array<Vertex, 3>> Graph::findAllTriangles() const {
  std::vector<std::array<Vertex, 3>> triangles;
  const std::size_t n = this->size();
  for (Vertex u = 0; u < n; ++u) {
    for (Vertex v : neighbours(u)) {
      if (v <= u)
        continue;
      for (Vertex w : neighbours(v)) {
        if (w <= v || w == u)
          continue;
        if (hasEdge(u, w)) {
          triangles.push_back({u, v, w});
        }
      }
    }
  }
  return triangles;
}
} // namespace algebra