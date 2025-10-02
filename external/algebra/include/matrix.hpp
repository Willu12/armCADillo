#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <stdexcept>
#include <vector>

#include "vec.hpp"

namespace algebra {

template <std::floating_point T, size_t Rows, size_t Cols> class Matrix {
private:
  std::array<std::array<T, Cols>, Rows> data;

public:
  inline Matrix() {
    for (size_t i = 0; i < Rows; i++)
      for (size_t j = 0; j < Cols; j++)
        data[i][j] = 0;
  }

  inline Matrix(const std::array<std::array<T, Cols>, Rows> &data)
      : data(data) {}

  inline T &operator()(size_t i, size_t j) { return data[i][j]; }
  inline const T &operator()(size_t i, size_t j) const { return data[i][j]; }

  /*
  inline Matrix operator*(const Matrix &m) const {
    Matrix result;
    for (size_t i = 0; i < Rows; i++)
      for (size_t j = 0; j < Cols; j++)
        for (size_t k = 0; k < Rows; k++)
          result(i, j) += data[i][k] * m(k, j);
    return result;
  };
  */
  template <size_t OtherCols>
  inline Matrix<T, Rows, OtherCols>
  operator*(const Matrix<T, Cols, OtherCols> &m) const {
    return multiply(*this, m);
  }

  inline Matrix operator*(const T &scalar) const {
    Matrix result;
    for (size_t i = 0; i < Rows; i++)
      for (size_t j = 0; j < Cols; j++)
        result(i, j) = data[i][j] * scalar;
    return result;
  }

  template <size_t Length>
    requires(Cols == Rows && Rows == Length)
  inline Vec<T, Length> operator*(const Vec<T, Length> &v) const {
    Vec<T, Rows> result;
    for (size_t i = 0; i < Rows; ++i) {
      result[i] = 0;
      for (size_t j = 0; j < Cols; ++j) {
        result[i] += data[i][j] * v[j];
      }
    }
    return result;
  }

  static inline Matrix Identity() {
    Matrix result;
    for (size_t i = 0; i < Rows; i++)
      result(i, i) = 1;
    return result;
  }

  inline Matrix<T, Cols, Rows> transpose() const {
    Matrix<T, Cols, Rows> result;
    for (size_t i = 0; i < Rows; i++) {
      for (size_t j = 0; j < Cols; j++) {
        result(j, i) = data[i][j];
      }
    }
    return result;
  }
  static inline Matrix diagonal(const std::vector<T> &diagonal) {
    if (diagonal.size() != Rows)
      throw std::invalid_argument("Invalid diagonal size");

    Matrix result;
    for (size_t i = 0; i < Rows; i++)
      result(i, i) = diagonal[i];
    return result;
  }

  static inline Matrix
  fromRows(const std::array<std::array<T, Cols>, Rows> &rows) {

    if (rows.size() != Rows) {
      throw std::invalid_argument("Invalid number of rows");
    }

    Matrix result;
    for (size_t i = 0; i < Rows; i++) {
      for (size_t j = 0; j < Cols; j++)
        result(i, j) = rows[i][j];
    }
    return result;
  }

  inline void print() const {
    for (size_t i = 0; i < Rows; i++) {
      for (size_t j = 0; j < Cols; j++)
        printf("%f ", data[i][j]);
      printf("\n");
    }
  }

  std::array<T, Cols> &operator[](size_t i) { return data[i]; }
  const std::array<T, Cols> &operator[](size_t i) const { return data[i]; }

  T &operator[](size_t i, size_t j) { return data[i][j]; }
  const T &operator[](size_t i, size_t j) const { return data[i][j]; }

  template <size_t M, size_t N, size_t P>
  static Matrix<T, M, P> multiply(const Matrix<T, M, N> &a,
                                  const Matrix<T, N, P> &b) {
    Matrix<T, M, P> result{};
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < P; j++) {
        T sum = T{};
        for (size_t k = 0; k < N; k++) {
          sum += a(i, k) * b(k, j);
        }
        result(i, j) = sum;
      }
    }
    return result;
  }

  template <size_t Length>
  static inline Matrix<T, Length, 1> fromVector(const Vec<T, Length> V) {

    Matrix<T, Length, 1> m;
    for (int i = 0; i < Length; ++i) {
      m[i][0] = V[i];
    }
    return m;
  }

  inline Vec<T, Rows> toVector() const {
    if (Cols > 1)
      throw std::runtime_error("Invalid col Count");

    Vec<T, Rows> v;
    for (int i = 0; i < Rows; ++i) {
      v[i] = (*this)(i, 0);
    }
    return v;
  }
};

template <typename T> using Mat4 = Matrix<T, 4, 4>;
template <typename T> using Mat3 = Matrix<T, 3, 3>;
template <typename T> using Mat2 = Matrix<T, 2, 2>;

using Mat4f = Mat4<float>;
using Mat3f = Mat3<float>;
using Mat2f = Mat2<float>;

using Mat4d = Mat4<double>;
using Mat3d = Mat3<double>;
using Mat2d = Mat2<double>;
} // namespace algebra