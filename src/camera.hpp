#pragma once
#include "glfwHelper.hpp"
#include "spherical.hpp"
#include "transformations.hpp"
#include "utils.hpp"

class Camera {
public:
  explicit Camera(GLFWwindow *window) : _window(window) {}

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

  algebra::Mat4f projectionMatrix() const {
    auto aspectRatio = GLFWHelper::getAspectRatio(_window);
    return algebra::transformations::projection(
        algebra::rotations::toRadians(_zoom), aspectRatio, 0.1f, 100.f);
  }

  algebra::Mat4f inverseProjectionMatrix() const {
    auto aspectRatio = GLFWHelper::getAspectRatio(_window);
    return algebra::transformations::inverseProjection(
        algebra::rotations::toRadians(_zoom), aspectRatio, 0.1f, 100.f);
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
    _target = _target + shiftWorld.fromHomogenous();
  }

  algebra::Vec3f getPosition() const { return _position.getCartesian(); }
  algebra::SphericalPosition<float> getSphericalPosition() const {
    return _position;
  }

  float &getZoom() { return _zoom; }
  const float &getZoom() const { return _zoom; }

  GLFWwindow &getWindow() const { return *_window; }

private:
  GLFWwindow *_window;
  algebra::SphericalPosition<float> _position =
      algebra::SphericalPosition((algebra::Vec3f(0.0f, 0.1f, -1.f)));
  algebra::Vec3f _target = algebra::Vec3f(0.f, 0.0f, 0.0f);
  algebra::Vec3f _up = algebra::Vec3f(0.f, 1.0f, 0.f);
  float _zoom = 45.f;
};