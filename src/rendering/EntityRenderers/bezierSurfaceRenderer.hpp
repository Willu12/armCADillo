#pragma once

#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "shader.hpp"
class BezierSurfaceRenderer : public IEntityRenderer {
public:
  explicit BezierSurfaceRenderer(const Camera &camera)
      : _shader("", ""), _camera(camera) {}

private:
  Shader _shader;
  const Camera &_camera;
};