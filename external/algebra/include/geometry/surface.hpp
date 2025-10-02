#pragma once

#include "../parametricForms/IDifferentialParametricForm.hpp"
#include <algorithm>
#include <cstdint>

namespace algebra {

static inline float bernstein(int i, int n, float t) {
  float coeff = 1.0f;
  for (int k = 0; k < i; ++k)
    coeff *= static_cast<float>(n - k) / (k + 1);
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
    const auto &P = patch.patch;
    const auto &uv = patch.localPos;

    algebra::Vec3f result{0.f, 0.f, 0.f};
    for (uint32_t i = 0; i < 4; ++i) {
      float bU = bernstein(i, 3, uv[1]);
      for (uint32_t j = 0; j < 4; ++j) {
        float bV = bernstein(j, 3, uv[0]);
        result = result + bU * bV * P[i][j];
      }
    }
    return result;
  }
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

    algebra::Matrix<float, 3, 2> J;
    J(0, 0) = du[0];
    J(1, 0) = du[1];
    J(2, 0) = du[2];

    J(0, 1) = dv[0];
    J(1, 1) = dv[1];
    J(2, 1) = dv[2];

    return J;
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

  LocalBezierPatch
  getCorrespondingBezierPatch(const algebra::Vec2f &pos) const {
    // Clamp to just below 1 to avoid falling out of bounds
    float clampedU = std::clamp(pos[0], 0.0f, 1.0f);

    float clampedV = std::clamp(pos[1], 0.0f, 1.0f);

    // Compute which patch we're in
    uint32_t colPatchIndex =
        std::min(static_cast<uint32_t>(clampedU * patches_.colCount),
                 patches_.colCount - 1);
    uint32_t rowPatchIndex =
        std::min(static_cast<uint32_t>(clampedV * patches_.rowCount),
                 patches_.rowCount - 1);

    // Compute local position within patch
    float patchUStart = static_cast<float>(colPatchIndex) / patches_.colCount;
    float patchVStart = static_cast<float>(rowPatchIndex) / patches_.rowCount;

    float localU = (clampedU - patchUStart) * patches_.colCount;
    float localV = (clampedV - patchVStart) * patches_.rowCount;

    // Extract 4x4 control points for the patch
    std::array<std::array<algebra::Vec3f, 4>, 4> patch;
    uint32_t u_offset = colPatchIndex * 3;
    uint32_t v_offset = rowPatchIndex * 3;
    uint32_t u_points = 3 * patches_.colCount + 1;

    for (uint32_t row = 0; row < 4; ++row) {
      for (uint32_t col = 0; col < 4; ++col) {
        uint32_t globalRow = v_offset + row;
        uint32_t globalCol = u_offset + col;
        patch[row][col] = points_[globalRow * u_points + globalCol];
      }
    }

    return LocalBezierPatch{.patch = patch, .localPos = {localU, localV}};
  }

  // private:
  std::vector<algebra::Vec3f> points_;
  ConnectionType connectionType_;
  Patches patches_ = Patches{.colCount = 1, .rowCount = 1};
};
} // namespace algebra