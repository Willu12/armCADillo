#pragma once

#include "bezierSurfaceC0.hpp"
#include "borderGraph.hpp"
#include "graph.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <array>
#include <functional>
#include <unordered_map>
#include <vector>

struct GregoryQuad {
  std::array<algebra::Vec3f, 4> top;
  std::array<algebra::Vec3f, 4> bottom;
  std::array<algebra::Vec3f, 2> topSides;
  std::array<algebra::Vec3f, 2> bottomSides;
  std::array<algebra::Vec3f, 4> uInner;
  std::array<algebra::Vec3f, 4> vInner;
};
class GregorySurface : public BezierSurface {
public:
  explicit GregorySurface(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces) {}

private:
  std::vector<std::array<algebra::Vec3f, 16>> calculateGregoryPoints();
  std::pair<algebra::Graph,
            std::unordered_map<std::size_t,
                               std::reference_wrapper<const PointEntity>>>
  createSurfacesGraph(const std::vector<std::reference_wrapper<BezierSurfaceC0>>
                          &surfaces) const;
  Border getBorder(const BezierSurfaceC0 &surface) const;

  BorderGraph createBorderGraph(
      const std::vector<std::reference_wrapper<BezierSurfaceC0>> &surfaces)
      const;
  BezierSurfaceC0 &
  findSurfaceForEdge(const PointEntity &p1, const PointEntity &p2,
                     const std::vector<std::reference_wrapper<BezierSurfaceC0>>
                         &surfaces) const;
  std::array<GregoryQuad, 3> GetGregoryQuadsFromEdges(
      const std::array<std::reference_wrapper<const Edge>, 3> &edges) const;
};