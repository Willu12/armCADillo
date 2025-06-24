#pragma once

#include "IEntityRenderer.hpp"
#include "bezierSurfaceC0.hpp"
#include "camera.hpp"
#include "glad/gl.h"
#include "mesh.hpp"
#include "pointRenderer.hpp"
#include "shader.hpp"
#include "surfaceMeshRenderer.hpp"

class BezierSurfaceRenderer : public IEntityRenderer {
public:
  explicit BezierSurfaceRenderer(const Camera &camera)
      : _shader({ShaderPath{._path = "../resources/shaders/vertexSurface.glsl",
                            ._type = GL_VERTEX_SHADER},
                 ShaderPath{._path = "../resources/shaders/"
                                     "surfaceTesselationControl.glsl",
                            ._type = GL_TESS_CONTROL_SHADER},
                 ShaderPath{._path = "../resources/shaders/"
                                     "surfaceIsolineTeselationEvaluation.glsl",
                            ._type = GL_TESS_EVALUATION_SHADER},

                 ShaderPath{
                     ._path = "../resources/shaders/fragmentShaderTrimmed.hlsl",
                     ._type = GL_FRAGMENT_SHADER}}),
        _meshRenderer(camera), _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());

    for (const auto &entity : entities) {
      auto &bezierSurface = dynamic_cast<BezierSurface &>(*entity);

      _shader.setUInt("u_patches", bezierSurface.getPatches().rowCount);
      _shader.setUInt("v_patches", bezierSurface.getPatches().colCount);
      _shader.setUInt("u_subdivisions", bezierSurface.getMeshDensity().s);
      _shader.setUInt("v_subdivisions", bezierSurface.getMeshDensity().t);
      _shader.setInt("renderPolyLine",
                     static_cast<int>(bezierSurface.wireframe()));
      const auto &mesh = bezierSurface.getMesh();
      if (bezierSurface.isTrimmed()) {
        _shader.setInt("trim", 1);
      } else {
        _shader.setInt("trim", 0);
      }
      if (bezierSurface.hasIntersectionTexture()) {
        const auto &texture = bezierSurface.getIntersectionTexutre();
        texture.bind();
      }
      glLineWidth(2.0f);

      glPatchParameteri(GL_PATCH_VERTICES, 16);
      glBindVertexArray(mesh.getVAO());
      _shader.setUInt("direction", 0);

      glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh.getIndicesLength()));

      _shader.setUInt("direction", 1);
      glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh.getIndicesLength()));
      glBindVertexArray(0);
    }
    _meshRenderer.render(entities);
  }

private:
  Shader _shader;
  SurfaceMeshRenderer _meshRenderer;
  const Camera &_camera;
};