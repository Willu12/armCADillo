#pragma once

#include "IEntity.hpp"
#include "imgui_stdlib.h"
#include "mesh.hpp"

class PointEntity : public IEntity {
public:
  PointEntity(algebra::Vec3f position) : _mesh(generateMesh()) {
    _name = "Point_" + std::to_string(_id++);
    _position = position;
    _meshKind = MeshKind::Triangles;
    _scale = 0.05f;
  }

  void updateMesh() override { _mesh = generateMesh(); };
  bool renderSettings() override {
    ImGui::InputText("Name", &getName());
    return false;
  }
  const Mesh &getMesh() const override { return *_mesh; }

private:
  inline static int _id;
  std::shared_ptr<Mesh> _mesh;

  std::shared_ptr<Mesh> generateMesh() {
    std::vector<float> vertices = {// Front face
                                   -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                                   0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

                                   // Back face
                                   -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
                                   0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
    std::vector<unsigned int> indices = {// Front edges
                                         0, 1, 2, 2, 3, 0,
                                         // right
                                         1, 5, 6, 6, 2, 1,

                                         // up
                                         2, 6, 7, 7, 3, 2,

                                         // Left edges
                                         7, 1, 5, 5, 4, 0,

                                         0, 4, 7, 7, 3, 0, 4, 5, 6, 6, 7, 4};
    return Mesh::create(vertices, indices);
  }
};