#pragma once
#include "matrix.hpp"
#include "mesh.hpp"

class IEntity {
public:
  virtual ~IEntity() = default;

  virtual algebra::Mat4f getModelMatrix() const = 0;
  virtual const Mesh &getMesh() const = 0;
};