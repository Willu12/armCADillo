#pragma once

#include "../matrix.hpp"
#include "../transformations.hpp"
#include "../vec.hpp"

namespace algebra {
template <std::floating_point T> class EulerAngle {
public:
  Mat4f getRotationMatrix() const {
    return transformations::rotationXMatrix(_rotation[0]) *
           transformations::rotationYMatrix(_rotation[1]) *
           transformations::rotationZMatrix(_rotation[2]);
  }
  void rotate(float x, float y, float z) {
    _rotation = _rotation + Vec3f(x, y, z);
  }
  Vec3f &getRotation() { return _rotation; }

private:
  Vec3f _rotation;
};
} // namespace algebra