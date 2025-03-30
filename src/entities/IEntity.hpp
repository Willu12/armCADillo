#pragma once
#include "IRenderable.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "quaternion.hpp"
#include "transformations.hpp"

class IEntity : public IRenderable {
public:
  virtual ~IEntity() = default;

  float &getScale() { return _scale; }
  const float &getScale() const { return _scale; }

  algebra::Vec3f &getPosition() override { return _position; }
  const algebra::Vec3f &getPosition() const override { return _position; }

  algebra::Quaternion<float> &getRotation() { return _rotation; }
  const algebra::Quaternion<float> &getRotation() const { return _rotation; }

  std::string &getName() { return _name; }
  const std::string &getName() const { return _name; }

  virtual void updateMesh() = 0;
  virtual bool renderSettings() = 0;

  algebra::Mat4f getModelMatrix() const {
    auto scale = getScale();
    auto scaleMatrix =
        algebra::transformations::scaleMatrix(scale, scale, scale);
    auto rotationMatrix = getRotation().getRotationMatrix();
    auto translationMatrix =
        algebra::transformations::translationMatrix(getPosition());

    return translationMatrix * rotationMatrix * scaleMatrix;
  }

protected:
  algebra::Vec3f _position;
  algebra::Quaternion<float> _rotation;
  float _scale = 1.f;
  std::string _name;
};