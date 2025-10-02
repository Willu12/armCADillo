#include <cmath>
#include <optional>

#include "../matrix.hpp"
#include "../vec.hpp"
namespace algebra {

class LinearSystem {
public:
  static std::optional<Vec4f> solveLinearSystem(const Mat4f &A_input,
                                                const Vec4f &b_input) {
    Mat4f A = A_input;
    Vec4f b = b_input;

    for (int i = 0; i < 4; ++i) {
      int maxRow = i;
      float maxVal = std::fabs(A[i, i]);
      for (int r = i + 1; r < 4; ++r) {
        float val = std::fabs(A[r, i]);
        if (val > maxVal) {
          maxVal = val;
          maxRow = r;
        }
      }
      if (maxVal < 1e-12f) {
        // Matrix is singular or near-singular
        return std::nullopt;
      }

      if (maxRow != i) {
        std::swap(A[i], A[maxRow]);
        std::swap(b[i], b[maxRow]);
      }

      for (int r = i + 1; r < 4; ++r) {
        float factor = A[r, i] / A[i, i];
        for (int c = i; c < 4; ++c) {
          A[r, c] -= factor * A[i, c];
        }
        b[r] -= factor * b[i];
      }
    }

    Vec4f x{};
    for (int i = 3; i >= 0; --i) {
      float sum = b[i];
      for (int c = i + 1; c < 4; ++c) {
        sum -= A[i, c] * x[c];
      }
      if (std::fabs(A[i, i]) < 1e-12f) {
        // Singular matrix
        return std::nullopt;
      }
      x[i] = sum / A[i, i];
    }

    return x;
  }
};
} // namespace algebra