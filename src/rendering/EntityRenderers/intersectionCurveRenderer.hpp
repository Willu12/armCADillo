#pragma once

#include "IEntityRenderer.hpp"
#include "camera.hpp"
#include "centerPointRenderer.hpp"
#include "intersectionCurve.hpp"
#include "shader.hpp"
class IntersectionCurveRenderer : public IEntityRenderer {
public:
  explicit IntersectionCurveRenderer(const Camera &camera)
      : _shader({ShaderPath{._path = "../resources/shaders/vertexMesh.hlsl",
                            ._type = GL_VERTEX_SHADER},
                 ShaderPath{._path =
                                "../resources/shaders/colorFragmentShader.hlsl",
                            ._type = GL_FRAGMENT_SHADER}}),
        _camera(camera) {
    firstPointRenderer_ = std::make_unique<CenterPointRenderer>(camera);
  }

  void render(const std::vector<std::shared_ptr<IEntity>> &entities) override {
    if (entities.empty())
      return;
    _shader.use();
    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());
    _shader.setVec4f("Color", _color);
    glLineWidth(3.0f);

    for (const auto &entity : entities) {
      const auto &mesh = entity->getMesh();
      glBindVertexArray(mesh.getVAO());
      glDrawElements(GL_LINES, mesh.getIndicesLength(), GL_UNSIGNED_INT,
                     nullptr);
      glBindVertexArray(0);
    }
    for (const auto &entity : entities) {
      auto &intersectionCurve = dynamic_cast<IntersectionCurve &>(*entity);
      firstPointRenderer_->render(intersectionCurve.firstPoint());
    }
    glLineWidth(2.0f);
  }

private:
  Shader _shader;
  std::unique_ptr<CenterPointRenderer> firstPointRenderer_;
  const Camera &_camera;
  algebra::Vec4f _color{0.9f, .0f, 0.0f, 1.f};
};