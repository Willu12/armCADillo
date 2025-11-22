#pragma once

#include "camera.hpp"
#include "namedPath.hpp"
#include "shader.hpp"

class MillingPathRenderer {
public:
  explicit MillingPathRenderer(const Camera *camera);
  void render(const NamedPath &millingPath);

private:
  Shader shader_;
  const Camera *camera_ = nullptr;
  algebra::Vec4f color_{0.0f, 1.0f, 0.0f, 1.f};
};