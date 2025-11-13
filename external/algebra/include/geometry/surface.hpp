#pragma once

#include "../parametricForms/IDifferentialParametricForm.hpp"
#include "../vec.hpp"
#include <algorithm>
#include <cstdint>

namespace algebra {

static inline float bernstein(int i, int n, float t) {
  float coeff = 1.0f;
  for (int k = 0; k < i; ++k) {
    coeff *= static_cast<float>(n - k) / static_cast<float>(k + 1);
  }
  return coeff * std::pow(t, i) * std::pow(1 - t, n - i);
}
struct Patches {
  uint32_t colCount;
  uint32_t rowCount;
};

enum class ConnectionType : uint8_t { Columns, Rows, Flat };

struct LocalBezierPatch {
  std::array<std::array<algebra::Vec3f, 4>, 4> patch;
  algebra::Vec2f localPos;
};

struct SecondDerivatives {
  Vec3f duu;
  Vec3f duv;
  Vec3f dvu;
  Vec3f dvv;
};

class BezierSurfaceC0 : public IDifferentialParametricForm<2, 3> {
public:
  BezierSurfaceC0(const std::vector<algebra::Vec3f> &points, uint32_t uCount,
                  uint32_t vCount, ConnectionType connectionType)
      : points_(points), connectionType_(connectionType) {

    if (connectionType == ConnectionType::Columns) {
      for (int i = 0; i < (3 * uCount + 1); ++i) {
        points_.push_back(points_[i]);
      }
    }
    patches_ = {.colCount = uCount, .rowCount = vCount};
  }

  std::array<algebra::Vec2f, 2> bounds() const override {
    return {algebra::Vec2f{0.f, 1.f}, algebra::Vec2f{0.f, 1.f}};
  }

  algebra::Vec3f value(const algebra::Vec2f &pos) const override {
    auto patch = getCorrespondingBezierPatch(pos);
    const auto &p = patch.patch;
    const auto &uv = patch.localPos;

    algebra::Vec3f result{0.f, 0.f, 0.f};
    for (uint32_t i = 0; i < 4; ++i) {
      float b_u = bernstein(i, 3, uv[1]);
      for (uint32_t j = 0; j < 4; ++j) {
        float b_v = bernstein(j, 3, uv[0]);
        result = result + b_u * b_v * p[i][j];
      }
    }
    return result;
  }

  // ---- First order derivatives -----------------
  std::pair<algebra::Vec3f, algebra::Vec3f>
  derivatives(const algebra::Vec2f &pos) const override {
    auto patch = getCorrespondingBezierPatch(pos);
    const auto &P = patch.patch;
    const auto &uv = patch.localPos;

    algebra::Vec3f dv{0.f, 0.f, 0.f}; // formerly 'du'
    algebra::Vec3f du{0.f, 0.f, 0.f}; // formerly 'dv'

    for (uint32_t i = 0; i < 3; ++i) {
      float Bv = bernstein(i, 2, uv[1]);
      for (uint32_t j = 0; j < 4; ++j) {
        float Bu = bernstein(j, 3, uv[0]);
        dv = dv + (P[i + 1][j] - P[i][j]) * (Bv * Bu);
      }
    }
    dv = dv * 3.f * static_cast<float>(patches_.rowCount);

    for (uint32_t i = 0; i < 4; ++i) {
      float Bv = bernstein(i, 3, uv[1]);
      for (uint32_t j = 0; j < 3; ++j) {
        float Bu = bernstein(j, 2, uv[0]);
        du = du + (P[i][j + 1] - P[i][j]) * (Bv * Bu);
      }
    }
    du = du * 3.f * static_cast<float>(patches_.colCount);
    return {du, dv};
  }

  algebra::Matrix<float, 3, 2>
  jacobian(const algebra::Vec2f &pos) const override {
    auto [du, dv] = derivatives(pos);

    algebra::Matrix<float, 3, 2> jacobian;
    jacobian(0, 0) = du[0];
    jacobian(1, 0) = du[1];
    jacobian(2, 0) = du[2];

    jacobian(0, 1) = dv[0];
    jacobian(1, 1) = dv[1];
    jacobian(2, 1) = dv[2];

    return jacobian;
  }

  // ----- Second order derivatives ---------------------
  SecondDerivatives secondDerivatives(const algebra::Vec2f &pos) const {
    auto local_patch = getCorrespondingBezierPatch(pos);
    const auto &patch = local_patch.patch;
    const auto &uv = local_patch.localPos;

    const float u = uv.x();
    const float v = uv.y();

    Vec3f duu;
    Vec3f duv;
    Vec3f dvu;
    Vec3f dvv;

    // ---- duu ----
    for (uint32_t i = 0; i < 4; ++i) {
      float b_v = bernstein(i, 3, v);
      for (uint32_t j = 0; j < 2; ++j) {
        float b_u = bernstein(j, 1, u);
        Vec3f delta = patch[i][j + 2] - 2.f * patch[i][j + 1] + patch[i][j];
        duu = duu + delta * (b_u * b_v);
      }
    }
    duu = duu * 3.f * 2.f *
          static_cast<float>(patches_.colCount * patches_.colCount);

    // dvv
    for (uint32_t i = 0; i < 2; ++i) {
      float b_v = bernstein(i, 1, v);
      for (uint32_t j = 0; j < 4; ++j) {
        float b_u = bernstein(j, 3, u);
        Vec3f delta = patch[i + 2][j] - 2.f * patch[i + 1][j] + patch[i][j];
        dvv = dvv + delta * (b_u * b_v);
      }
    }
    dvv = dvv * 3.f * 2.f *
          static_cast<float>(patches_.rowCount * patches_.rowCount);

    // ---- duv
    for (uint32_t i = 0; i < 3; ++i) {
      float b_v = bernstein(i, 2, v);
      for (uint32_t j = 0; j < 3; ++j) {
        float b_u = bernstein(j, 2, u);
        Vec3f delta = patch[i + 1][j + 1] - patch[i + 1][j] - patch[i][j + 1] +
                      patch[i][j];
        duv = duv + delta * (b_u * b_v);
      }
    }

    duv = duv * 3.f * 3.f *
          static_cast<float>(patches_.colCount * patches_.rowCount);
    dvu = duv;

    return {.duu = duu, .duv = duv, .dvu = dvu, .dvv = dvv};
  }

  bool wrapped(size_t dim) const override {
    if (dim == 0) {
      return connectionType_ == ConnectionType::Columns;
    }
    if (dim == 1) {
      return connectionType_ == ConnectionType::Rows;
    }
    return false;
  }

  Vec3f normal(const Vec2f &pos) const {
    auto deriv = derivatives(pos);
    const Vec3f &du = deriv.first;
    const Vec3f &dv = deriv.second;
    return du.cross(dv).normalize();
  }

  LocalBezierPatch
  getCorrespondingBezierPatch(const algebra::Vec2f &pos) const {
    float u = std::clamp(pos[0], 0.0f, 1.0f);
    float v = std::clamp(pos[1], 0.0f, 1.0f);

    uint32_t patch_column_index = std::min(
        static_cast<uint32_t>(u * patches_.colCount), patches_.colCount - 1);
    uint32_t patch_row_index = std::min(
        static_cast<uint32_t>(v * patches_.rowCount), patches_.rowCount - 1);

    float patch_u_start =
        static_cast<float>(patch_column_index) / patches_.colCount;
    float patch_v_start =
        static_cast<float>(patch_row_index) / patches_.rowCount;

    float u_local = (u - patch_u_start) * patches_.colCount;
    float v_local = (v - patch_v_start) * patches_.rowCount;

    std::array<std::array<algebra::Vec3f, 4>, 4> patch;
    uint32_t u_offset = patch_column_index * 3;
    uint32_t v_offset = patch_row_index * 3;
    uint32_t u_points = 3 * patches_.colCount + 1;

    for (uint32_t row = 0; row < 4; ++row) {
      for (uint32_t col = 0; col < 4; ++col) {
        uint32_t global_row = v_offset + row;
        uint32_t global_col = u_offset + col;
        patch[row][col] = points_[global_row * u_points + global_col];
      }
    }

    return LocalBezierPatch{.patch = patch, .localPos = {u_local, v_local}};
  }

  // private:
  std::vector<algebra::Vec3f> points_;
  ConnectionType connectionType_;
  Patches patches_ = Patches{.colCount = 1, .rowCount = 1};
};
} // namespace algebra