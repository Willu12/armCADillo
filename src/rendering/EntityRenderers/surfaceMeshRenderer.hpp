#pragma once

#include "IEntityRenderer.hpp"
#include "bezierSurfaceC0.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "pointRenderer.hpp"
#include "shader.hpp"
#include "vec.hpp"

class SurfaceMeshRenderer : public IEntityRenderer {
public:
  explicit SurfaceMeshRenderer(const Camera &camera)
      : _shader({ShaderPath{._path = "../resources/shaders/vertexMesh.hlsl",
                            ._type = GL_VERTEX_SHADER},
                 ShaderPath{._path =
                                "../resources/shaders/colorFragmentShader.hlsl",
                            ._type = GL_FRAGMENT_SHADER}}),
        _camera(camera) {}

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty()) {
      return;
    }
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());
    _shader.setVec4f("Color", _color);

    for (const auto &entity : entities) {
      auto &bezierSurface = dynamic_cast<BezierSurface &>(*entity);
      if (!bezierSurface.wireframe()) {
        continue;
      }
      const auto &mesh = bezierSurface.getPolyMesh();
      // glLineWidth(2.0f);

      glBindVertexArray(mesh.getVAO());

      // glDrawArrays(GL_LINES, 0, static_cast<int>(mesh.getIndicesLength()));
      glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT,
                     nullptr);

      glBindVertexArray(0);
    }
  }

private:
  Shader _shader;
  const Camera &_camera;
  algebra::Vec4f _color{0.6f, .1f, 0.6f, 1.f};
};