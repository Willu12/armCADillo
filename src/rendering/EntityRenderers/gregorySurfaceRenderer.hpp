#pragma once

#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "gregorySurface.hpp"
#include "surfaceMeshRenderer.hpp"
#include <ranges>
class GregorySurfaceRenderer : public IEntityRenderer {
public:
  explicit GregorySurfaceRenderer(const Camera &camera)
      : _shader({ShaderPath{._path = "../resources/shaders/vertexSurface.glsl",
                            ._type = GL_VERTEX_SHADER},
                 ShaderPath{._path = "../resources/shaders/"
                                     "surfaceTesselationControl.glsl",
                            ._type = GL_TESS_CONTROL_SHADER},
                 ShaderPath{._path = "../resources/shaders/"
                                     "surfaceIsolineTeselationEvaluation.glsl",
                            ._type = GL_TESS_EVALUATION_SHADER},

                 ShaderPath{._path = "../resources/shaders/fragmentShader.hlsl",
                            ._type = GL_FRAGMENT_SHADER}}),
        _meshRenderer(camera), _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());

    for (const auto &entity : entities) {
      auto &gregorySurface = dynamic_cast<GregorySurface &>(*entity);

      _shader.setUInt("u_subdivisions", 4);
      _shader.setUInt("v_subdivisions", 4);
      //_shader.setInt("renderPolyLine",
      //               static_cast<int>(bezierSurface.wireframe()));
      glLineWidth(2.0f);
      glPointSize(10.0f);

      for (const auto &[i, mesh] :
           gregorySurface.getMeshes() | std::views::enumerate) {

        // if (i != 2)
        //  continue;
        glPatchParameteri(GL_PATCH_VERTICES, 16);
        glBindVertexArray(mesh->getVAO());
        _shader.setUInt("direction", 0);
        glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh->getIndicesLength()));
        //  glDrawArrays(GL_POINTS, 0,
        //  static_cast<int>(mesh->getIndicesLength()));

        _shader.setUInt("direction", 1);
        glDrawArrays(GL_PATCHES, 0, static_cast<int>(mesh->getIndicesLength()));
        glBindVertexArray(0);
      }
    }
    // _meshRenderer.render(entities);
  }

private:
  Shader _shader;
  SurfaceMeshRenderer _meshRenderer;
  const Camera &_camera;
};