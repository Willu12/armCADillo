#pragma once
#include "IEntity.hpp"
#include "shader.hpp"
#include "transformations.hpp"
#include "vec.hpp"

class Cursor : public IEntity {
public:
  Cursor() : _mesh(generateMesh()) {}
  algebra::Mat4f getModelMatrix() const override {
    return algebra::transformations::translationMatrix(_position);
  }

  const Mesh &getMesh() const override { return *_mesh; }
  const algebra::Vec3f &getPosition() const override { return _position; }

  void updatePosition(float x, float y, float aspectRatio,
                      const Camera &camera) {
    // get current position
    auto projection = algebra::transformations::projection(aspectRatio);
    auto sceneCursorPosition =
        projection * (camera.viewMatrix() * _position.toHomogenous());

    sceneCursorPosition = sceneCursorPosition * (1.0f / sceneCursorPosition[3]);
    float z_ndc = sceneCursorPosition[2];
    float z_screen =
        (z_ndc + 1.0f) * 0.5f; // Convert NDC [-1,1] to depth range [0,1]

    auto screenPosition = algebra::Vec3f(x, y, z_ndc).toHomogenous();

    auto viewPosition =
        algebra::transformations::inverseProjection(aspectRatio) *
        screenPosition;
    viewPosition = viewPosition * (1. / viewPosition[3]);

    auto worldPos = camera.inverseViewMatrix() * (viewPosition);

    _position = viewPosition.fromHomogenous();
    printf("x == %f, y== %f, z==%f w==%f\n", worldPos[0], worldPos[1],
           worldPos[2], worldPos[3]);
  }

private:
  algebra::Vec3f _position = algebra::Vec3f(0.0f, 0.0f, 0.0f);
  float _radius = 0.05f;
  std::shared_ptr<Mesh> _mesh;
  GLFWwindow *_window;

  std::shared_ptr<Mesh> generateMesh() {
    std::vector<float> vertices = {// left down             //text
                                   -_radius, -_radius, 0.f, 0.f, 0.f,
                                   // left up               //text
                                   -_radius, _radius, 0.f, 0.f, 1.0,
                                   // right up             //text
                                   _radius, _radius, 0.f, 1.f, 1.f,
                                   // right down             //text
                                   _radius, -_radius, 0.f, 1.f, 0.f};
    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};

    return Mesh::createTexturedMesh(vertices, indices);
  }
};