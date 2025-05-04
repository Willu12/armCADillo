#pragma once

#include "IEntity.hpp"
#include "ISubscribable.hpp"
#include "IVisitor.hpp"
#include "mesh.hpp"

class VirtualPoint : public IEntity, public ISubscribable {
public:
  explicit VirtualPoint(algebra::Vec3f position) : _mesh(generateMesh()) {
    _name = "BezierPoint" + std::to_string(_id++);
    _position = position;
    _meshKind = MeshKind::Triangles;
    _scale = 0.02f;
  }

  bool acceptVisitor(IVisitor &visitor) override {
    return visitor.visitVirtualPoint(*this);
  }

  void notifySubscribers() override;

  void updatePosition(const algebra::Vec3f &position) override {
    notifySubscribers();
    IEntity::updatePosition(position);
  }

  const algebra::Vec3f &getPosition() const override { return _position; }

  void updateMesh() override { _mesh = generateMesh(); };
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