#pragma once
#include "IRenderable.hpp"
#include "IVisitor.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"

#include "matrix.hpp"
#include "quaternion.hpp"
#include "transformations.hpp"
#include "vec.hpp"

class GUI;

class IEntity : public IRenderable {
public:
  virtual ~IEntity() = default;
  virtual void updateMesh() = 0;
  virtual bool acceptVisitor(IVisitor &visitor);
  virtual bool renderSettings(const GUI &gui);
  virtual void rotateAroundPoint(const algebra::Quaternion<float> &rotation,
                                 const algebra::Vec3f &point);
  virtual void scaleAroundPoint(float scaleFactor,
                                const algebra::Vec3f &centerPoint);

  algebra::Vec3f &getScale();
  const algebra::Vec3f &getScale() const;

  void updatePosition(const algebra::Vec3f &position) override;
  const algebra::Vec3f &getPosition() const override;

  algebra::Quaternion<float> &getRotation();
  const algebra::Quaternion<float> &getRotation() const;

  std::string &getName();
  const std::string &getName() const;

  algebra::Mat4f getModelMatrix() const override;

  const uint32_t &getId() const;
  uint32_t &getId();
  const bool &dirty() const;
  bool &dirty();

protected:
  algebra::Vec3f _position;
  algebra::Quaternion<float> _rotation;
  algebra::Vec3f _scale = algebra::Vec3f(1.f, 1.f, 1.f);
  std::string _name;
  bool _dirty = false;
  uint32_t _id;
};