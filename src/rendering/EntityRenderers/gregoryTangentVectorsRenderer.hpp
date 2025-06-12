#pragma once

#include "IEntityRenderer.hpp"
#include "bezierSurfaceC0.hpp"
#include "camera.hpp"
#include "glad/gl.h"
#include "gregorySurface.hpp"
#include "mesh.hpp"
#include "pointRenderer.hpp"
#include "shader.hpp"
#include "vec.hpp"
#include <ranges>

class GregoryTangentVectorsRenderer : public IEntityRenderer {
public:
  explicit GregoryTangentVectorsRenderer(const Camera &camera)
      : _shader({ShaderPath{._path = "../resources/shaders/vertexMesh.hlsl",
                            ._type = GL_VERTEX_SHADER},
                 ShaderPath{._path =
                                "../resources/shaders/colorFragmentShader.hlsl",
                            ._type = GL_FRAGMENT_SHADER}}),
        _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());
    _shader.setVec4f("Color", _color);

    for (const auto &entity : entities) {
      auto &gregorySurface = dynamic_cast<GregorySurface &>(*entity);
      if (!gregorySurface.showTangentVectors())
        return;
      glLineWidth(2.0f);

      const auto &meshes = gregorySurface.getTangentMeshes();
      for (const auto &[i, mesh] : meshes | std::views::enumerate) {
        if (i > 0)
          continue;
        glBindVertexArray(mesh->getVAO());
        glDrawElements(GL_LINES, mesh->getIndicesLength(), GL_UNSIGNED_INT,
                       nullptr);
      }

      glBindVertexArray(0);
    }
  }

private:
  Shader _shader;
  const Camera &_camera;
  algebra::Vec4f _color{0.1f, .5f, 0.3f, 1.f};
};