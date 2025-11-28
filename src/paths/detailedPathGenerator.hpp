#pragma once

#include "GCodeSerializer.hpp"
#include "bezierSurface.hpp"
#include "cutter.hpp"
#include "heightMap.hpp"
#include "intersectionCurve.hpp"
#include "intersectionFinder.hpp"
#include "intersectionTexture.hpp"
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
  void setModel(const Model *model) { model_ = model; }
  void setCutter(Cutter cutter) { cutter_ = cutter; }
  void setScene(Scene *scene) { scene_ = scene; }
  void setIntersectionFinder(IntersectionFinder *intersectionFinder) {
    intersectionFinder_ = intersectionFinder;
  }
  void setHeightMap(const HeightMap *heightMap) { heightMap_ = heightMap; }

  const std::vector<IntersectionCurve *> &intersections() const {
    return intersections_;
  }

  void colorSegments(BezierSurface &surface,
                     const std::vector<std::vector<Coord>> &segments);

  Direction &direction() { return direction_; }
  int &lines() { return lines_; }

  void generatePathForIntersectionCurve(
      const IntersectionCurve &intersectionCurve) const;

private:
  const Model *model_ = nullptr;
  Cutter cutter_;
  Scene *scene_ = nullptr;
  std::vector<IntersectionCurve *> intersections_;
  IntersectionFinder *intersectionFinder_;
  const HeightMap *heightMap_;

  int lines_ = 40;
  Direction direction_ = Direction::Vertical;

  void setFloorAsTrimmed(BezierSurface &intersectableSurface) const;

  std::vector<std::vector<Coord>> generateLineSegments(BezierSurface &surface);

  std::vector<std::vector<algebra::Vec3f>>
  generateSurfacePaths(const BezierSurface &surface,
                       std::vector<std::vector<Coord>> &segments) const;

  std::vector<algebra::Vec3f> generateLinePoints(const BezierSurface &surface,
                                                 Coord start, Coord end) const;

  std::vector<algebra::Vec3f> combineSurfacePaths(
      const std::vector<std::vector<algebra::Vec3f>> &surfacePaths) const;

  std::vector<Coord> shortestPathAlongIntersectioCurve(
      Coord start, Coord end, const IntersectionTexture &intersectionTexture,
      Direction direction) const;

  std::vector<algebra::Vec3f>
  safeConnectionBridge(Coord start, Coord end,
                       const BezierSurface &surface) const;

  bool intersects(Coord coord,
                  const IntersectionTexture &intersectionTexture) const;
  void fixIntersectionLine(std::vector<algebra::Vec3f> &points,
                           const BezierSurface &surface) const;
  std::vector<algebra::Vec3f>
  createSafeBridge(Coord start, Coord end, const BezierSurface &surface) const;
};

namespace std {
template <> struct hash<DetailedPathGenerator::Coord> {
  std::size_t operator()(const DetailedPathGenerator::Coord &c) const {
    // Compute individual hashes
    auto h1 = std::hash<int>{}(c.x);
    auto h2 = std::hash<int>{}(c.y);

    // Combine them using a standard bit-shifting method
    // (Prevents (1,0) and (0,1) from having the same hash)
    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};
} // namespace std