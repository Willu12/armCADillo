#pragma once

#include "intersectionTexture.hpp"
class Intersectable {
public:
  virtual ~Intersectable() = default;
  bool hasIntersectionTexture() { return intersectionTexture_ != nullptr; }
  const IntersectionTexture &getIntersectionTexture() const {
    return *intersectionTexture_;
  };

  IntersectionTexture &getIntersectionTexture() {
    return *intersectionTexture_;
  }

  void setIntersectionTexture(IntersectionTexture *intersectionTexture) {
    intersectionTexture_ = intersectionTexture;
  }

  bool &isTrimmed() { return isTrimmed_; }
  const bool &isTrimmed() const { return isTrimmed_; }

private:
  IntersectionTexture *intersectionTexture_ = nullptr;
  bool isTrimmed_ = false;
};