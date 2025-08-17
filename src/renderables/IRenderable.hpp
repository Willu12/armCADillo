#pragma once
#include "IMeshable.hpp"
#include "color.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
enum class MeshKind : uint8_t { Lines = 0, Triangles = 1 };

class IRenderable {
public:
  virtual ~IRenderable() = default;
  virtual algebra::Mat4f getModelMatrix() const = 0;
  virtual const IMeshable &getMesh() const = 0;
  virtual const algebra::Vec3f &getPosition() const = 0;
  virtual void updatePosition(const algebra::Vec3f &position) = 0;
  const Color &getColor() { return _color; }
  void setColor(const Color &color) { _color = color; }

  //  virtual algebra::Vec3f &getPosition() = 0;

  MeshKind getMeshKind() const { return _meshKind; }

protected:
  MeshKind _meshKind = MeshKind::Lines;
  Color _color = Color::White();
};