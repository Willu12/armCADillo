#pragma once

#include "intersectionTexture.hpp"
class Intersectable {
public:
  virtual ~Intersectable() = default;
  bool hasIntersectionTexture() const {
    return intersectionTexture_ != nullptr;
  }
  const IntersectionTexture &getIntersectionTexture() const {
    return *intersectionTexture_;
  };

  IntersectionTexture *getIntersectionTexture() { return intersectionTexture_; }

  void setIntersectionTexture(IntersectionTexture *intersectionTexture) {
    intersectionTexture_ = intersectionTexture;
  }

  void combineIntersectionTexture(IntersectionTexture *intersectionTexture) {
    if (intersectionTexture_ == nullptr) {
      setIntersectionTexture(intersectionTexture);
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
  }

  bool &isTrimmed() { return isTrimmed_; }
  const bool &isTrimmed() const { return isTrimmed_; }

private:
  IntersectionTexture *intersectionTexture_ = nullptr;
  bool isTrimmed_ = false;
};