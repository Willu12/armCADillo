#pragma once

#include "matrix.hpp"
#include "vec.hpp"
#include <concepts>

namespace algebra {
namespace transformations {

template <std::floating_point T>
inline Mat4<T> lookAt(const Vec3<T> &observation, const Vec3<T> &camera,
                      const Vec3<T> &up) {
  Vec3<T> to_camera = (camera - observation).normalize(); // direction
  Vec3<T> right = up.cross(to_camera).normalize();        // camera x axis
  Vec3<T> head = to_camera.cross(right);                  // camera up axis

  std::array<std::array<T, 4>, 4> rows = {
      {{right[0], right[1], right[2], -right.dot(camera)},
       {head[0], head[1], head[2], -head.dot(camera)},
       {to_camera[0], to_camera[1], to_camera[2], -to_camera.dot(camera)},
       {0, 0, 0, 1}}};

  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T>
inline Mat4<T> inverseLookAt(const Vec3<T> &observation, const Vec3<T> &camera,
                             const Vec3<T> &up) {
  Vec3<T> to_camera = (camera - observation).normalize();
  Vec3<T> right = up.cross(to_camera).normalize();
  Vec3<T> head = to_camera.cross(right);

  // Transpose of rotation matrix
  std::array<std::array<T, 4>, 4> rows = {
      {{right[0], head[0], to_camera[0], camera[0]},
       {right[1], head[1], to_camera[1], camera[1]},
       {right[2], head[2], to_camera[2], camera[2]},
       {0, 0, 0, 1}}};

  return Mat4<T>::fromRows(rows);
}

template <typename T>
inline Mat4<T> projectionOffCenter(const T left, const T right, const T bottom,
                                   const T top, const T nearPlane,
                                   const T farPlane) {
  const T rl = right - left;
  const T tb = top - bottom;
  const T fn = farPlane - nearPlane;

  std::array<std::array<T, 4>, 4> rows = {
      {{2 * nearPlane / rl, 0, (right + left) / rl, 0},
       {0, 2 * nearPlane / tb, (top + bottom) / tb, 0},
       {0, 0, -(farPlane + nearPlane) / fn, -2 * farPlane * nearPlane / fn},
       {0, 0, -1, 0}}};

  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T>
inline Mat4<T> projection(const T fov, const T aspectRatio, const T nearPlane,
                          const T farPlane) {
  const T ctgHalfFov = 1 / std::tan(fov / 2);
  const T viewDistance = farPlane - nearPlane;

  std::array<std::array<T, 4>, 4> rows = {
      {{ctgHalfFov / aspectRatio, 0, 0, 0},
       {0, ctgHalfFov, 0, 0},
       {0, 0, -(farPlane + nearPlane) / viewDistance,
        -2 * farPlane * nearPlane / viewDistance},
       {0, 0, -1, 0}}};

  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T>
inline Mat4<T> projection(const T aspectRatio) {
  return projection(1.3f, aspectRatio, 0.1f, 100.f);
}

template <std::floating_point T>
inline Mat4<T> inverseProjection(const T fov, const T aspectRatio,
                                 const T nearPlane, const T farPlane) {
  const T viewDistance = farPlane - nearPlane;

  std::array<std::array<T, 4>, 4> rows = {
      {{aspectRatio * std::tan(fov / 2), 0, 0, 0},
       {0, std::tan(fov / 2), 0, 0},
       {0, 0, 0, -1},
       {0, 0, viewDistance / (-2 * (farPlane * nearPlane)),
        -(farPlane + nearPlane) / viewDistance *
            (viewDistance / (-2 * (farPlane * nearPlane)))}}};

  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T>
inline Mat4<T> inverseProjection(const T aspectRatio) {
  return inverseProjection(1.3f, aspectRatio, 0.1f, 100.f);
}

template <std::floating_point T>
inline Mat4<T> inverseView(const Mat4<T> &view) {
  Mat4<T> inverseView = view.transpose();
  inverseView(0, 3) = -view(0, 0) * view(0, 3) - view(1, 0) * view(1, 3) -
                      view(2, 0) * view(2, 3);
  inverseView(1, 3) = -view(0, 1) * view(0, 3) - view(1, 1) * view(1, 3) -
                      view(2, 1) * view(2, 3);
  inverseView(2, 3) = -view(0, 2) * view(0, 3) - view(1, 2) * view(1, 3) -
                      view(2, 2) * view(2, 3);
  return inverseView;
}

template <std::floating_point T> inline Mat4<T> scaleMatrix(T sx, T sy, T sz) {
  std::array<std::array<T, 4>, 4> rows = {
      {{sx, 0, 0, 0}, {0, sy, 0, 0}, {0, 0, sz, 0}, {0, 0, 0, 1}}};
  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T> inline Mat4<T> scaleMatrix(Vec3<T> scale) {
  return scaleMatrix(scale[0], scale[1], scale[2]);
}

template <std::floating_point T> inline Mat4<T> rotationZMatrix(T angle) {
  const T c = std::cos(angle);
  const T s = std::sin(angle);

  std::array<std::array<T, 4>, 4> rows = {
      {{c, -s, 0, 0}, {s, c, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T> inline Mat4<T> rotationXMatrix(T angle) {
  const T c = std::cos(angle);
  const T s = std::sin(angle);

  std::array<std::array<T, 4>, 4> rows = {
      {{1, 0, 0, 0}, {0, c, -s, 0}, {0, s, c, 0}, {0, 0, 0, 1}}};
  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T> inline Mat4<T> rotationYMatrix(T angle) {
  const T c = std::cos(angle);
  const T s = std::sin(angle);

  std::array<std::array<T, 4>, 4> rows = {
      {{c, 0, s, 0}, {0, 1, 0, 0}, {-s, 0, c, 0}, {0, 0, 0, 1}}};
  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T>
inline Mat4<T> translationMatrix(T tx, T ty, T tz) {
  std::array<std::array<T, 4>, 4> rows = {
      {{1, 0, 0, tx}, {0, 1, 0, ty}, {0, 0, 1, tz}, {0, 0, 0, 1}}};
  return Mat4<T>::fromRows(rows);
}

template <std::floating_point T>
inline Mat4<T> translationMatrix(Vec3<T> translation) {
  return translationMatrix(translation[0], translation[1], translation[2]);
}
} // namespace transformations
} // namespace algebra