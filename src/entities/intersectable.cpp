#include "intersectable.hpp"
#include "color.hpp"
#include "intersectionTexture.hpp"
#include <algorithm>
#include <cstdint>

void Intersectable::combineIntersectionTexture(
    IntersectionTexture *intersectionTexture) {
  if (intersectionTexture_ == nullptr) {
    setIntersectionTexture(intersectionTexture);
    return;
  }

  auto size = intersectionTexture->getSize();

  for (int y = 0; y < size.height; ++y) {
    for (int x = 0; x < size.width; ++x) {
      auto new_cell_type = intersectionTexture->getCellType(x, y);
      if (new_cell_type == IntersectionTexture::CellType::Intersection) {
        intersectionTexture_->setCellType(
            x, y, IntersectionTexture::CellType::Intersection);
      }
    }
  }

  intersectionTexture_->update();
}

void Intersectable::combineAndConnectIntersectionTexture(
    IntersectionTexture *intersectionTexture) {
  if (intersectionTexture_ == nullptr) {
    setIntersectionTexture(intersectionTexture);
    return;
  }
  /// find start and begin of current intersection
  auto &current_curve = intersectionTexture_->getIntersectionCurve().back();
  auto &next_curve = intersectionTexture->getIntersectionCurve().front();

  auto fix_intersections = [&](IntersectionTexture::Segment &curr,
                               IntersectionTexture::Segment &next) {
    const bool first_rotatable =
        intersectionTexture_->getIntersectionCurve().size() == 1;
    const auto width = intersectionTexture_->getSize().width;
    const auto height = intersectionTexture->getSize().height;

    auto dist_squared_wrapped = [&](const IntersectionTexture::Coord &a,
                                    const IntersectionTexture::Coord &b) {
      uint32_t dx = a.x > b.x ? a.x - b.x : b.x - a.x;
      uint32_t dy = a.y > b.y ? a.y - b.y : b.y - a.y;

      if (intersectionTexture->wrapU()) {
        auto dx_wrap = width - dx;
        dx = std::min(dx_wrap, dx);
      }

      if (intersectionTexture->wrapV()) {
        auto dy_wrap = height - dy;
        dy = std::min(dy_wrap, dy);
      }

      return dx * dx + dy * dy;
    };

    auto d_es = dist_squared_wrapped(curr.end, next.start);
    auto d_ee = dist_squared_wrapped(curr.end, next.end);
    auto d_ss = dist_squared_wrapped(curr.start, next.start);
    auto d_se = dist_squared_wrapped(curr.start, next.end);

    auto min_d = std::min({d_es, d_ee, d_ss, d_se});

    if (min_d == d_ee) {
      std::swap(next.start, next.end);
    } else if (min_d == d_ss && first_rotatable) {
      std::swap(curr.start, curr.end);
    } else if (min_d == d_se && first_rotatable) {
      std::swap(curr.start, curr.end);
      std::swap(next.start, next.end);
    }
  };

  fix_intersections(current_curve, next_curve);

  combineIntersectionTexture(intersectionTexture);
  auto curr_end_uv =
      intersectionTexture->uv(current_curve.end.x, current_curve.end.y);
  auto next_start_uv =
      intersectionTexture->uv(next_curve.start.x, next_curve.start.y);

  // auto middle_point = (curr_end_uv + next_start_uv) * 0.5f;

  // intersectionTexture_->drawLine({curr_end_uv, middle_point});
  intersectionTexture_->drawLine({curr_end_uv, next_start_uv}, Color::Red());

  intersectionTexture_->update();

  auto &curve = intersectionTexture_->getIntersectionCurve();
  curve.push_back(next_curve);
  /// add line betwen current and prev;
}