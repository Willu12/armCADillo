#pragma once
#include "../vec.hpp"
#include <random>
namespace algebra {

class Distribution {
public:
  static int randomInt(int start, int end) {
    static thread_local std::mt19937 gen(
        std::random_device{}());
    std::uniform_int_distribution<> distr(start, end);
    return distr(gen);
  }
  static algebra::Vec3f randomPointInsideSphere(const Vec3f &center,
                                                float radius) {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    while (true) {
      float x = dist(rng);
      float y = dist(rng);
      float z = dist(rng);

      float lenSq = x * x + y * y + z * z;
      if (lenSq <= 1.0f) {
        float scale = std::cbrt(dist01(rng));
        return center + algebra::Vec3f{x, y, z}.normalize() * (radius * scale);
      }
    }
  }
};
} // namespace algebra
