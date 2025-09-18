#pragma once

#include "IMeshable.hpp"
#include "vec.hpp"
#include <cstdio>
#include <memory>

struct BezierVertex {
  int len;
  algebra::Vec3f pts[4];
};

class BezierMesh : public IMeshable {
public:
  uint32_t getVAO() const override;
  uint32_t getVBO() const override;
  uint32_t getEBO() const override;
  uint32_t getIndicesLength() const override;

  static std::unique_ptr<BezierMesh>
  create(const std::vector<algebra::Vec3f> &vertices);

  static std::unique_ptr<BezierMesh>
  createC2(const std::vector<algebra::Vec3f> &vertices);

  ~BezierMesh() override;

  BezierMesh(BezierMesh &&other) noexcept;
  BezierMesh &operator=(BezierMesh &&other) noexcept;

  BezierMesh(const BezierMesh &) = delete;
  BezierMesh &operator=(const BezierMesh &) = delete;

protected:
  explicit BezierMesh(const std::vector<algebra::Vec3f> &vertices);

private:
  uint32_t _vao, _vbo, _ebo;
  std::vector<BezierVertex> _bezierSegments;

  void addSimpleVertexLayout();
  void initBuffers();

  static std::vector<BezierVertex>
  buildBezierVertexData(const std::vector<algebra::Vec3f> &points);
  static std::vector<BezierVertex>
  bezierC2Data(const std::vector<algebra::Vec3f> &points);
};