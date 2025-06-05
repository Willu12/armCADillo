#pragma once

#include "bezierSurfaceC0.hpp"
#include "graph.hpp"
#include "pointEntity.hpp"
#include "vec.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
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
  std::vector<std::reference_wrapper<const PointEntity>>
  getBorder(const BezierSurfaceC0 &surface) const;
};