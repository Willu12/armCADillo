#pragma once

#include "IEntityRenderer.hpp"
#include "bezierSurfaceC0.hpp"
#include "camera.hpp"
#include "glad/gl.h"
#include "mesh.hpp"
#include "pointRenderer.hpp"
#include "shader.hpp"

class BezierSurfaceRenderer : public IEntityRenderer {
public:
  explicit BezierSurfaceRenderer(const Camera &camera)
      : _shader({ShaderPath{._path = "../resources/shaders/vertexSurface.glsl",
                            ._type = GL_VERTEX_SHADER},
                 ShaderPath{._path = "../resources/shaders/"
                                     "surfaceTesselationControl.glsl",
                            ._type = GL_TESS_CONTROL_SHADER},
                 ShaderPath{._path = "../resources/shaders/"
                                     "surfaceTesselationEvaluation.glsl",
                            ._type = GL_TESS_EVALUATION_SHADER},

                 ShaderPath{._path = "../resources/shaders/fragmentShader.hlsl",
                            ._type = GL_FRAGMENT_SHADER}}),
        _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.projectionMatrix());

    for (const auto &entity : entities) {
      auto &bezierSurface = dynamic_cast<BezierSurface &>(*entity);

      _shader.setUInt("u_subdivisions", bezierSurface.getMeshDensity().s);
      _shader.setUInt("v_subdivisions", bezierSurface.getMeshDensity().t);
      _shader.setUInt("u_patches", bezierSurface.getPatches().sCount);
      _shader.setUInt("v_patches", bezierSurface.getPatches().tCount);

      const auto &mesh = bezierSurface.getMesh();
      glLineWidth(2.0f);
      if (bezierSurface.wireframe())
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glPatchParameteri(GL_PATCH_VERTICES, 16);
      glBindVertexArray(mesh.getVAO());

      glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh.getIndicesLength()));
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBindVertexArray(0);
    }
  }

private:
  Shader _shader;
  const Camera &_camera;
};