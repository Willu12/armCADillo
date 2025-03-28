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
  algebra::Vec3f &getPosition() override { return _position; }
  const algebra::Vec3f &getPosition() const override { return _position; }

  algebra::EulerAngle<float> &getRotation() override { return _rotation; }
  float &getScale() override { return _scale; }

  void updatePosition(float x, float y, const Camera &camera) {
    auto zoom_rad = algebra::rotations::toRadians(camera.getZoom());

    auto projection = camera.projectionMatrix();
    auto sceneCursorPosition =
        projection * (camera.viewMatrix() * _position.toHomogenous());

    sceneCursorPosition = sceneCursorPosition * (1.0f / sceneCursorPosition[3]);
    float z_ndc = sceneCursorPosition[2];

    auto screenPosition = algebra::Vec3f(x, y, z_ndc).toHomogenous();

    auto viewPosition = camera.inverseProjectionMatrix() * screenPosition;
    viewPosition = viewPosition * (1.f / viewPosition[3]);
    auto worldPos = camera.inverseViewMatrix() * viewPosition;
    _position = worldPos.fromHomogenous();
  }

private:
  algebra::Vec3f _position = algebra::Vec3f(0.0f, 0.0f, 0.0f);
  algebra::EulerAngle<float> _rotation;
  float _radius = 0.03f;
  float _scale = 1.f;
  std::shared_ptr<Mesh> _mesh;

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