#pragma once
#include "../matrix.hpp"
#include <concepts>

namespace algebra {

template <std::floating_point T> class Quaternion {
public:
  Quaternion(T w, T x, T y, T z) : _w(w), _x(x), _y(y), _z(z) {}
  Quaternion() : _w(1.), _x(0), _y(0), _z(0) {}

  static Quaternion fromAxisAngle(const Vec3f &axis, T angle) {
    auto normalizedAxis = axis.normalize();
    T halfAngle = angle / T(2.0);
    T s = std::sin(halfAngle);
    T c = std::cos(halfAngle);
    return Quaternion(c, normalizedAxis[0] * s, normalizedAxis[1] * s,
                      normalizedAxis[2] * s);
  }

  Quaternion operator+(const Quaternion &q) const {
    return Quaternion(_w + q._w, _x + q._x, _y + q._y, _z + q._z);
  }
  Quaternion operator*(const Quaternion &q) const {
    return Quaternion(_w * q._w - _x * q._x - _y * q._y - _z * q._z,
                      _w * q._x + _x * q._w + _y * q._z - _z * q._y,
                      _w * q._y - _x * q._z + _y * q._w + _z * q._x,
                      _w * q._z + _x * q._y - _y * q._x + _z * q._w);
  }
  Quaternion conjugate() const { return Quaternion(_w, -_x, -_y, -_z); }

  T norm() const { return std::sqrt(_w * _w + _x * _x + _y * _y + _z * _z); }

  Quaternion normalized() const {
    T n = norm();
    if (n > 0 == false)
      throw std::runtime_error("Cannot normalize a quaternion with zero norm.");

    return Quaternion(_w / n, _x / n, _y / n, _z / n);
  }

  Quaternion inverse() const {
    T n = _w * _w + _x * _x + _y * _y + _z * _z;
    if (n == 0) {
      throw std::runtime_error("Cannot invert a quaternion with zero norm.");
    }
    return Quaternion(_w / n, -_x / n, -_y / n, -_z / n);
  }

  Mat4<T> getRotationMatrix() const {
    T xx = _x * _x, yy = _y * _y, zz = _z * _z;
    T xy = _x * _y, xz = _x * _z, yz = _y * _z;
    T wx = _w * _x, wy = _w * _y, wz = _w * _z;

    return Mat4<T>({{{1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 0},
                     {2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx), 0},
                     {2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy), 0},
                     {0, 0, 0, 1}}});
  }

  Quaternion operator*(const Vec3<T> &point) const {
    auto purePoint = point.toQuaternion();
    auto conjugate = this->conjugate();
    auto conjugated = purePoint * conjugate;
    return *this * conjugated;
  }

  T &x() { return _x; }
  T &y() { return _y; }
  T &z() { return _z; }
  T &w() { return _w; }
  const T &x() const { return _x; }
  const T &y() const { return _y; }
  const T &z() const { return _z; }
  const T &w() const { return _w; }

  Vec3<T> toVector() const { return algebra::Vec3<T>(_x, _y, _z); }

private:
  T _w, _x, _y, _z;
};
}; // namespace algebra