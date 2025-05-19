#pragma once

#include "IEntityRenderer.hpp"
#include "bezierSurface.hpp"
#include "camera.hpp"
#include "glad/gl.h"
#include "mesh.hpp"
#include "pointRenderer.hpp"
#include "shader.hpp"

class BezierSurfaceC2Renderer : public IEntityRenderer {
public:
  explicit BezierSurfaceC2Renderer(const Camera &camera)
      : _shader(
            {ShaderPath{._path = "../resources/shaders/vertexSurface.glsl",
                        ._type = GL_VERTEX_SHADER},
             ShaderPath{
                 ._path = "../resources/shaders/surfaceTesselationControl.glsl",
                 ._type = GL_TESS_CONTROL_SHADER},
             ShaderPath{
                 ._path =
                     "../resources/shaders/surfaceC2TesselationEvaluation.glsl",
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
      _shader.setUInt("v_patches", bezierSurface.getPatches().tCount);
      _shader.setUInt("u_patches", bezierSurface.getPatches().sCount);

      const auto &mesh = bezierSurface.getMesh();
      glLineWidth(2.0f);
      if (bezierSurface.wireframe())
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glPatchParameteri(GL_PATCH_VERTICES, 16);
      glBindVertexArray(mesh.getVAO());
      // _shader.setUInt("direction", 0);

      // glUniform1ui(direction_loc, 0); // linie wzdłuż v
      glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh.getIndicesLength()));

      // _shader.setUInt("direction", 1);
      // glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh.getIndicesLength()));
      //  glDrawArrays(GL_PATCHES, 0,
      //  static_cast<int>(mesh.getIndicesLength()));
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBindVertexArray(0);
    }
  }

private:
  Shader _shader;
  const Camera &_camera;
};