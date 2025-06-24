#pragma once

#include "intersectionTexture.hpp"
#include <memory>
class Intersectable {
public:
  virtual ~Intersectable() = default;
  bool hasIntersectionTexture() { return !intersectionTexture_.expired(); }
  const IntersectionTexture &getIntersectionTexutre() {
    return *intersectionTexture_.lock();
  };

  void setIntersectionTexture(
      std::weak_ptr<IntersectionTexture> intersectionTexture) {
    intersectionTexture_ = intersectionTexture;
  }

  bool &isTrimmed() { return isTrimmed_; }
  const bool &isTrimmed() const { return isTrimmed_; }

private:
  std::weak_ptr<IntersectionTexture> intersectionTexture_;
  bool isTrimmed_ = false;
};