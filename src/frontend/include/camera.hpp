#pragma once
#include "algebra.hpp"
#include "spherical.hpp"

class Camera {
public:
  algebra::Mat4f viewMatrix() const {
    return algebra::transformations::lookAt(_target, _position.getCartesian(),
                                            _up);
  }

  void rotateHorizontal(float angle) {
    if (abs(_position._phi + angle) < M_PI_2)
      _position._phi += angle;
    return;
  }

  void rotateVertical(float angle) {
    if (abs(_position._theta + angle) < M_PI_2)
      _position._theta += angle;
    return;
  }

private:
  algebra::SphericalPosition<float> _position =
      algebra::SphericalPosition((algebra::Vec3f(0.0f, 5.0f, -5.f)));
  algebra::Vec3f _target = algebra::Vec3f(0.f, 0.0f, 0.0f);
  algebra::Vec3f _up = algebra::Vec3f(0.f, 1.0f, 0.f);
};