#pragma once

#include "GCodeSerializer.hpp"
#include "bezierSurface.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "intersectionCurve.hpp"
#include "intersectionFinder.hpp"
#include "model.hpp"
#include "normalOffsetSurface.hpp"
#include "scene.hpp"
#include "vec.hpp"
#include <memory>
#include <vector>
class DetailedPathGenerator {
public:
  struct Coord {
    int x;
    int y;

    bool operator==(const Coord &other) const {
      return x == other.x && y == other.y;
    }
  };

  enum class Direction : uint8_t { Vertical, Horizontal };

  void generate();
  void setModel(const Model *model) {
    model_ = model;
    offset_surfaces_.resize(model->surfaces().size());
  }
  void setCutter(Cutter cutter) { cutter_ = cutter; }
  void setScene(Scene *scene) { scene_ = scene; }
  void setIntersectionFinder(IntersectionFinder *intersectionFinder) {
    intersectionFinder_ = intersectionFinder;
  }

  const std::vector<IntersectionCurve *> &intersections() const {
    return intersections_;
  }

  void colorSegments(BezierSurface &surface,
                     const std::vector<std::vector<Coord>> &segments);

  Direction &direction() { return direction_; }
  int &lines() { return lines_; }

private:
  const Model *model_ = nullptr;
  Cutter cutter_;
  Scene *scene_ = nullptr;
  std::vector<IntersectionCurve *> intersections_;
  IntersectionFinder *intersectionFinder_;

  int lines_ = 200;
  Direction direction_ = Direction::Horizontal;

  std::vector<std::unique_ptr<algebra::NormalOffsetSurface>> offset_surfaces_;

  void setFloorAsTrimmed(BezierSurface &intersectableSurface) const;

  std::vector<std::vector<Coord>> generateLineSegments(BezierSurface &surface);

  std::vector<std::vector<algebra::Vec3f>>
  generateSurfacePaths(const BezierSurface &surface,
                       std::vector<std::vector<Coord>> &segments) const;

  std::vector<algebra::Vec3f> generateLinePoints(const BezierSurface &surface,
                                                 Coord start, Coord end) const;

  std::vector<algebra::Vec3f> combineSurfacePaths(
      const std::vector<std::vector<algebra::Vec3f>> &surfacePaths) const;
};