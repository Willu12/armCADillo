#pragma once
#include "matrix.hpp"
#include "mesh.hpp"

class IRenderable {
public:
  virtual ~IRenderable() = default;
  virtual algebra::Mat4f getModelMatrix() const = 0;
  virtual const Mesh &getMesh() const = 0;
  virtual const algebra::Vec3f &getPosition() const = 0;
  virtual algebra::Vec3f &getPosition() = 0;
};