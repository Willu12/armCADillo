#pragma once

#include <array>
#include <cmath>
#include <vector>

template <std::floating_point T, size_t Rows, size_t Cols> class Matrix;

namespace algebra {

template <std::floating_point T> class Quaternion;

template <std::floating_point T, size_t Length> class Vec {
public:
  Vec() : values{} {}
  explicit Vec(const std::array<T, Length> &vals) : values(vals) {}
  template <typename... Args>
    requires(sizeof...(Args) == Length)
  Vec(Args... args) : values{static_cast<T>(args)...} {}

  T &operator[](size_t index) { return values[index]; }
  const T &operator[](size_t index) const { return values[index]; }

  Vec operator+(const Vec &other) const {
    Vec result;
    for (size_t i = 0; i < Length; ++i)
      result[i] = values[i] + other[i];

    return result;
  }
  Vec operator-(const Vec &other) const {
    Vec result;
    for (size_t i = 0; i < Length; ++i)
      result[i] = values[i] - other[i];

    return result;
  }

  Vec operator*(const T scalar) const {
    Vec result;
    for (size_t i = 0; i < Length; ++i)
      result[i] = values[i] * scalar;

    return result;
  }

  Vec operator/(const T scalar) const {
    Vec result;
    for (size_t i = 0; i < Length; ++i)
      result[i] = values[i] / scalar;

    return result;
  }

  T dot(const Vec &other) const {
    T sum = 0;
    for (size_t i = 0; i < Length; ++i)
      sum += values[i] * other[i];

    return sum;
  }

  Vec cross(const Vec &other) const
    requires(Length == 3)
  {
    Vec result;
    result[0] = values[1] * other[2] - values[2] * other[1];
    result[1] = values[2] * other[0] - values[0] * other[2];
    result[2] = values[0] * other[1] - values[1] * other[0];
    return result;
  }

  T length() const { return sqrt(this->dot(*this)); }

  Vec normalize() const {
    T len = length();
    return (*this) * (len == 0 ? 0 : (1 / len));
  }

  std::vector<T> toVector() const {
    std::vector<T> vector(Length);
    for (int i = 0; i < Length; i++) {
      vector[i] = values[i];
    }
    return vector;
  }

  Vec<T, Length + 1> toHomogenous() const {
    Vec<T, Length + 1> result;
    for (size_t i = 0; i < Length; ++i) {
      result[i] = values[i];
    }
    result[Length] = static_cast<T>(1);
    return result;
  }

  Vec<T, Length - 1> fromHomogenous() const {
    Vec<T, Length - 1> result;
    for (size_t i = 0; i < Length - 1; ++i) {
      result[i] = values[i];
    }
    return result;
  }

  Quaternion<T> toQuaternion() const {
    return Quaternion<T>(0, values[0], values[1], values[2]);
  }

private:
  std::array<T, Length> values;
};
template <typename T> using Vec3 = Vec<T, 3>;
template <typename T> using Vec4 = Vec<T, 4>;
template <typename T> using Vec2 = Vec<T, 2>;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

template <typename T, size_t Length>
Vec<T, Length> operator*(const T scalar, const Vec<T, Length> &vec) {
  return vec * scalar;
}

} // namespace algebra