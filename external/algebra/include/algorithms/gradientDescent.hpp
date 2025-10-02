#pragma once

#include "../functions.hpp"
#include "../vec.hpp"
#include <cstddef>
#include <memory>
#include <print>
#include <utility>
namespace algebra {

template <size_t SIZE> class GradientDescent {
public:
  explicit GradientDescent(
      std::unique_ptr<IDifferentiableScalarFunction<SIZE>> function)
      : function_(std::move(function)) {}

  float getLearningRate() const { return learningRate_; }
  void setIterationCount(std::size_t iterationCount) {
    iterationCount_ = iterationCount;
  }
  void setLearningRate(float learningRate) { learningRate_ = learningRate; }
  void setStopEpsilon(float stopEpsilon) { stopEpsilon_ = stopEpsilon; }
  void setStartingPoint(const Vec<float, SIZE> &startingPoint) {
    startingPoint_ = startingPoint;
  }

  std::optional<Vec<float, SIZE>> calculate() {
    auto arg = startingPoint_;

    auto val = function_->value(arg);

    for (int i = 0; i < iterationCount_; ++i) {
      auto newArg = arg - learningRate_ * function_->gradient(arg);

      for (size_t dim = 0; dim < SIZE; ++dim) {
        if (function_->wrapped(dim)) {
          float lower = function_->bounds()[dim][0];
          float upper = function_->bounds()[dim][1];
          float range = upper - lower;
          newArg[dim] = std::fmod(newArg[dim] - lower, range);
          if (newArg[dim] < 0) {
            newArg[dim] += range;
          }
          newArg[dim] += lower;
        } else {
          newArg[dim] = std::clamp(newArg[dim], function_->bounds()[dim][0],
                                   function_->bounds()[dim][1]);
        }
      }
      auto newVal = function_->value(newArg);

      if (std::abs((newVal - val)) < stopEpsilon_) {
        if (function_->same()) {
          algebra::Vec2f uv(newArg[0], newArg[1]);
          algebra::Vec2f zy(newArg[2], newArg[3]);
          auto paramDist = paramDistSquared(uv, zy);
          std::println("param dist squared == {}", paramDist);
          if (paramDistSquared(uv, zy) < 1e-2f) {
            std::println("params too close");
            return std::nullopt;
          }
        }
        std::println("gradient descend returns after {} iterations", i);
        return newArg;
      }
      if (newVal > val) {
        learningRate_ /= 2.f;
        continue;
      }

      arg = newArg;
      val = newVal;
    }
    if (function_->same()) {
      algebra::Vec2f uv(arg[0], arg[1]);
      algebra::Vec2f zy(arg[2], arg[3]);
      auto paramDist = paramDistSquared(uv, zy);
      std::println("param dist squared == {}", paramDist);
      if (paramDistSquared(uv, zy) < 1e-2f) {
        std::println("params too close");
        return std::nullopt;
      }
    }
    return arg;
  }

private:
  std::size_t iterationCount_ = 1000;
  float stopEpsilon_ = 1e-11;
  float learningRate_ = 0.001f;
  Vec<float, SIZE> startingPoint_;
  std::unique_ptr<IDifferentiableScalarFunction<SIZE>> function_;

  float paramDistSquared(const algebra::Vec2f &a, const algebra::Vec2f &b) {
    float du = std::fabs(a[0] - b[0]);
    float dv = std::fabs(a[1] - b[1]);

    if (function_->wrapped(0)) {
      float range = function_->bounds()[0][1] - function_->bounds()[0][0];
      du = std::min(du, range - du);
    }
    if (function_->wrapped(1)) {
      float range = function_->bounds()[1][1] - function_->bounds()[1][0];
      dv = std::min(dv, range - dv);
    }

    return du * du + dv * dv;
  }

  // bool parametersTooClose()
};
} // namespace algebra