#pragma once
#include "algebra.hpp"
#include "spherical.hpp"

class Camera {
public:
  algebra::Mat4f viewMatrix() const {
    auto Tx =
        algebra::transformations::translationMatrix(0.f, 0.f, -_position._r);
    auto R = _position.getRotationMatrix();
    auto Tp = algebra::transformations::translationMatrix(
        -_target[0], -_target[1], -_target[2]);

    return Tx * R * Tp;
  }

  algebra::Mat4f inverseViewMatrix() const {
    auto Tx =
        algebra::transformations::translationMatrix(0.f, 0.f, _position._r);
    auto R = _position.getRotationMatrix();
    auto Tp = algebra::transformations::translationMatrix(
        _target[0], _target[1], _target[2]);

    return Tp * R.transpose() * Tx;
  }

  void rotateHorizontal(float angle) {
    if (std::abs(_position._phi + angle) < M_PI_2)
      _position._phi += angle;
  }

  void rotateVertical(float angle) {
    if (std::abs(_position._theta + angle) < M_PI_2)
      _position._theta += angle;
  }

  void changeZoom(float zoom) {
    if (_position._r + zoom < 45.f && _position._r + zoom > 1.f)
      _position._r += zoom;
  }

  void updateTarget(float xShift, float yShift) {
    algebra::Vec4f shiftVector(xShift, yShift, 0.0f, 0.0f);
    auto shiftWorld = inverseViewMatrix() * shiftVector;
    _target =
        _target + algebra::Vec3f(shiftWorld[0], shiftWorld[1], shiftWorld[2]);
  }

  algebra::Vec3f getPosition() const { return _position.getCartesian(); }
  algebra::SphericalPosition<float> getSphericalPosition() const {
    return _position;
  }

private:
  algebra::SphericalPosition<float> _position =
      algebra::SphericalPosition((algebra::Vec3f(0.0f, 0.1f, -1.f)));
  algebra::Vec3f _target = algebra::Vec3f(0.f, 0.0f, 0.0f);
  algebra::Vec3f _up = algebra::Vec3f(0.f, 1.0f, 0.f);
};