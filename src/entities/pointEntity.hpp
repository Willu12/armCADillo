#pragma once

#include "IEntity.hpp"
#include "ISubscribable.hpp"
#include "IVisitor.hpp"
#include "mesh.hpp"
#include "vec.hpp"
#include <iostream>

class PointEntity : public IEntity, public ISubscribable {
public:
  explicit PointEntity(algebra::Vec3f position) : _mesh(generateMesh()) {
    _id = kClassId++;
    _name = "Point_" + std::to_string(_id);
    _position = position;
    _meshKind = MeshKind::Triangles;
    _scale = algebra::Vec3f(0.03f, 0.03f, 0.03f);
  }

  ~PointEntity() {
    for (const auto &subscriber : _subscribers) {
      subscriber.get().onSubscribableDestroyed(*this);
    }
  }
  bool acceptVisitor(IVisitor &visitor) override {
    bool changed = visitor.visitPoint(*this);
    if (changed) {
      notifySubscribers();
    }
    return changed;
  }

  void updatePosition(const algebra::Vec3f &position) override {
    notifySubscribers();
    IEntity::updatePosition(position);
  }

  void setPositionWithoutNotify(const algebra::Vec3f &position) {
    IEntity::updatePosition(position);
  }

  void rotateAroundPoint(const algebra::Quaternion<float> &rotation,
                         const algebra::Vec3f &point) override {
    notifySubscribers();
    IEntity::rotateAroundPoint(rotation, point);
  }

  void scaleAroundPoint(float scaleFactor,
                        const algebra::Vec3f &centerPoint) override {
    notifySubscribers();
    IEntity::scaleAroundPoint(scaleFactor, centerPoint);
  }

  // algebra::Vec3f &getPosition() override {
  //   notifySubscribers();
  //  return _position;
  // }
  const algebra::Vec3f &getPosition() const override { return _position; }

  void updateMesh() override { _mesh = generateMesh(); };
  const Mesh &getMesh() const override { return *_mesh; }
  bool &surfacePoint() { return _surfacePoint; }

  bool operator==(const PointEntity &other) const {
    return other._id == this->_id;
  }

  struct RefHash {
    std::size_t
    operator()(const std::reference_wrapper<PointEntity> &ref) const noexcept {
      return std::hash<PointEntity *>{}(&ref.get());
    }
  };

  struct RefEq {
    bool
    operator()(const std::reference_wrapper<PointEntity> &lhs,
               const std::reference_wrapper<PointEntity> &rhs) const noexcept {
      return &lhs.get() == &rhs.get();
    }
  };

private:
  inline static int kClassId;
  std::shared_ptr<Mesh> _mesh;
  bool _surfacePoint = false;

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