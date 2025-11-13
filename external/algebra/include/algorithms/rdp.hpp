#pragma once

#include "../vec.hpp"
#include <algorithm>
#include <vector>
namespace algebra {
class RDP {
public:
  std::vector<algebra::Vec3f> reducePoints(std::vector<Vec3f> &points,
                                           float eps) {
    auto first = points.front();
    auto last = points.back();

    points.pop_back();
    points.erase(points.begin());

    auto distance_from_segment = [](const Vec3f &p, const Vec3f &a,
                                    const Vec3f &b) {
      auto ab = b - a;
      auto ap = p - a;
      auto t = std::clamp(ap.dot(ab) / ab.dot(ab), 0.f, 1.f);

      auto closest = a + t * ab;
      return (closest - p).length();
    };

    auto p_m = std::ranges::min_element(
        points, [&first, &last, &distance_from_segment](const Vec3f &p,
                                                        const Vec3f &q) {
          auto p_distace = distance_from_segment(p, first, last);
          auto q_distance = distance_from_segment(q, first, last);
          return p_distace < q_distance;
        });

    auto d_min = distance_from_segment(*p_m, first, last);

    std::vector<Vec3f> result;
    if (d_min < eps) {
      result.push_back(first);
      result.push_back(last);
      return result;
    }

    result.push_back(first);

    ///
    std::vector<algebra::Vec3f> first_part;
    std::vector<algebra::Vec3f> second_part;

    first_part.insert(first_part.end(), points.begin(), p_m + 1);
    second_part.insert(second_part.end(), p_m, points.end());

    auto first_reduced = reducePoints(first_part, eps);
    auto second_reduced = reducePoints(second_part, eps);
  }

private:
};
} // namespace algebra