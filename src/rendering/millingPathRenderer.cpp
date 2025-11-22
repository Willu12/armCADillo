#include "millingPathRenderer.hpp"
#include "namedPath.hpp"
#include "shader.hpp"

MillingPathRenderer::MillingPathRenderer(const Camera *camera)
    : shader_({ShaderPath{._path = "../../resources/shaders/vertexMesh.hlsl",
                          ._type = GL_VERTEX_SHADER},
               ShaderPath{
                   ._path = "../../resources/shaders/colorFragmentShader.hlsl",
                   ._type = GL_FRAGMENT_SHADER}}),
      camera_(camera) {}

void MillingPathRenderer::render(const NamedPath &millingPath) {
  shader_.use();
  shader_.setViewMatrix(camera_->viewMatrix());
  shader_.setProjectionMatrix(camera_->getProjectionMatrix());
  shader_.setVec4f("Color", color_);

  const auto &mesh = millingPath.mesh();
  glBindVertexArray(mesh.getVAO());
  glDrawElements(GL_LINES, static_cast<GLsizei>(mesh.getIndicesLength()),
                 GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}