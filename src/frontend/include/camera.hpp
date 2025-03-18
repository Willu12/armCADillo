#pragma once
#include "algebra.hpp"

class Camera {
public:
  algebra::Mat4f viewMatrix() const {
    return algebra::transformations::lookAt(_target, _position, _up);
  }

private:
  algebra::Vec3f _position = algebra::Vec3f(0.0f, 0.0f, -3.f);
  algebra::Vec3f _target = algebra::Vec3f(0.f, 0.0f, 0.0f);
  algebra::Vec3f _up = algebra::Vec3f(0.f, 1.0f, 0.f);
};