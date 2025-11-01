#pragma once

#include "IRenderable.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"
class CenterPointRenderer {
public:
  explicit CenterPointRenderer(const Camera &camera)
      : _camera(camera),
        _shader("../../resources/shaders/textureShader.vert",
                "../../resources/shaders/texturedBillboardShader.frag"),
        _texture(Texture::createTexture(
            "../../resources/textures/ballTexture.png")) {}

  void render(const IRenderable &renderable) {
    _texture->bind(0);
    _shader.use();

    _shader.setViewMatrix(_camera.viewMatrix());
    _shader.setModelMatrix(
        renderable.getModelMatrix() *
        _camera.getSphericalPosition().getRotationMatrix().transpose());
    _shader.setProjectionMatrix(_camera.getProjectionMatrix());

    const auto &mesh = renderable.getMesh();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(mesh.getVAO());
    glDrawElements(GL_TRIANGLES, mesh.getIndicesLength(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
  }

private:
  const Camera &_camera;
  Shader _shader;
  std::unique_ptr<Texture> _texture;
};