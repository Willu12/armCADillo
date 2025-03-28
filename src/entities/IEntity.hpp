#pragma once
#include "IRenderable.hpp"
#include "matrix.hpp"
#include "mesh.hpp"

class IEntity : public IRenderable {
public:
  virtual ~IEntity() = default;

  virtual algebra::Vec3f &getPosition() = 0;

  virtual algebra::EulerAngle<float> &getRotation() = 0;
  virtual float &getScale() = 0;
  virtual void updateMesh() = 0;

  // MAYBE CHANGE IT LATER???
  virtual bool renderSettings() = 0;
};