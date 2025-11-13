#pragma once

#include "../geometry/plane.hpp"
#include "../vec.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

namespace algebra {

class RDP {
public:
  static std::vector<Vec3f> reducePoints(const std::vector<Vec3f> &points,
                                         float eps, Plane plane) {
    if (points.size() < 2) {
      return points;
    }

    return rdpRecursion(points, 0, points.size() - 1, eps, plane);
  }

private:
  static std::vector<Vec3f> rdpRecursion(const std::vector<Vec3f> &points,
                                         size_t start, size_t end, float eps,
                                         Plane plane) {
    const auto &first = points[start];
    const auto &last = points[end];

    if (end - start < 2) {
      if (end - start == 0) {
        return {first};
      }
      return {first, last};
    }

    auto project = [plane](const Vec3f &p) -> Vec2f {
      switch (plane) {
      case Plane::XY:
        return {p.x(), p.y()};
      case Plane::XZ:
        return {p.x(), p.z()};
      case Plane::YZ:
        return {p.y(), p.z()};
      }

      std::unreachable();
    };

    auto distance_from_segment = [&project](const Vec3f &p, const Vec3f &a,
                                            const Vec3f &b) {
      auto p_projected = project(p);
      auto a_projected = project(a);
      auto b_projected = project(b);
      auto ab = b_projected - a_projected;
      auto ap = p_projected - a_projected;
      auto t = std::clamp(ap.dot(ab) / ab.dot(ab), 0.f, 1.f);
      auto closest = a_projected + t * ab;
      return (closest - p_projected).length();
    };

    auto furthest_point =
        std::max_element(points.begin() + start + 1, points.begin() + end,
                         [&](const Vec3f &p, const Vec3f &q) {
                           return distance_from_segment(p, first, last) <
                                  distance_from_segment(q, first, last);
                         });

    float max_distance = distance_from_segment(*furthest_point, first, last);

    if (max_distance < eps) {
      return {first, last};
    }

    size_t furthest_index = std::distance(points.begin(), furthest_point);
    auto first_part = rdpRecursion(points, start, furthest_index, eps, plane);
    auto second_part = rdpRecursion(points, furthest_index, end, eps, plane);

    std::vector<Vec3f> result;
    result.reserve(first_part.size() + second_part.size());
    result.insert(result.end(), first_part.begin(), first_part.end() - 1);
    result.insert(result.end(), second_part.begin(), second_part.end());

    return result;
  }
};

} // namespace algebra
