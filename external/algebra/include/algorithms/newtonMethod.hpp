#pragma once

#include "../functions.hpp"
#include "../vec.hpp"
#include "linearSystem.hpp"
#include <cstddef>
#include <memory>
#include <print>
#include <stdexcept>

namespace algebra {

template <size_t IN, size_t OUT> class NewtonMethod {
public:
  NewtonMethod(std::unique_ptr<IDifferentialParametricForm<IN, OUT>> function,
               const Vec4f &startingPoint)
      : function_(std::move(function)), currentPoint(startingPoint) {}

  void setIterationCount(size_t iterations) { iterationCount_ = iterations; }
  std::optional<Vec4f> calculate() {
    auto arg = currentPoint;
    auto bounds = function_->bounds();

    for (size_t i = 0; i < iterationCount_; ++i) {
      auto jacobian = function_->jacobian(arg);
      auto valueVec = -1.0f * function_->value(arg);
      auto deltaOpt = LinearSystem::solveLinearSystem(jacobian, valueVec);
      // auto deltaOpt = std::nullopt;
      if (!deltaOpt) {
        return std::nullopt;
      }
      //  return std::nullopt;

      Vec4f delta = *deltaOpt;

      float damping = 1.0f;
      Vec4f newArg;
      while (damping > 1e-4f) {
        newArg = arg + damping * delta;

        // Clamp / wrap bounds
        for (std::size_t dim = 0; dim < 4; ++dim) {
          if (function_->wrapped(dim)) {
            float lower = bounds[dim][0];
            float upper = bounds[dim][1];
            float range = upper - lower;
            newArg[dim] = std::fmod(newArg[dim] - lower, range);
            if (newArg[dim] < 0)
              newArg[dim] += range;
            newArg[dim] += lower;
          } else {
            newArg[dim] =
                std::clamp(newArg[dim], bounds[dim][0], bounds[dim][1]);
          }
        }

        float oldResidual = function_->value(arg).length();
        float newResidual = function_->value(newArg).length();

        if (newResidual < oldResidual) {
          break;
        }
        damping *= 0.5f;
      }
      if ((function_->value(newArg) - function_->value(arg)).length() <
          kAccuracy) {
        std::println("Newton Returns after {} iterations", i);
        return newArg;
      }
      arg = newArg;
    }
    std::println("Newton  fail maxPreccsion == {}",
                 pow(function_->value(arg).length(), 2));
    return std::nullopt;
  }

private:
  std::unique_ptr<IDifferentialParametricForm<IN, OUT>> function_;

  Vec4f currentPoint;
  size_t iterationCount_ = 40;
  static constexpr float kAccuracy = 10e-6;
};
} // namespace algebra