#pragma once
#include "IEntity.hpp"
#include "camera.hpp"
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
  // algebra::Vec3f &getPosition() override { return _position; }

  void updatePosition(float x, float y, float z) {
    _position = algebra::Vec3f(x, y, z);
  }

  void updatePosition(float x, float y, const Camera &camera) {
    auto projection = camera.projectionMatrix();
    auto scene_cursor_pos =
        projection * (camera.viewMatrix() * _position.toHomogenous());

    scene_cursor_pos = scene_cursor_pos * (1.0f / scene_cursor_pos[3]);
    float z_ndc = scene_cursor_pos[2];

    auto screen_position = algebra::Vec3f(x, y, z_ndc).toHomogenous();

    auto view_position = camera.inverseProjectionMatrix() * screen_position;
    view_position = view_position * (1.f / view_position[3]);

    auto world_position = camera.inverseViewMatrix() * view_position;
    _position = world_position.fromHomogenous();
  }

  void updateMesh() override { _mesh = generateMesh(); }

private:
  algebra::Vec3f _position = algebra::Vec3f(0.0f, 0.0f, 0.0f);
  float _radius = 0.025f;
  std::unique_ptr<Mesh> _mesh;

  std::unique_ptr<Mesh> generateMesh() {
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