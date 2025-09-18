#pragma once

#include "IMeshable.hpp"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

class BezierSurfaceMesh : public IMeshable {
public:
  uint32_t getVAO() const override;
  uint32_t getVBO() const override;
  uint32_t getEBO() const override;
  uint32_t getIndicesLength() const override;

  static std::unique_ptr<BezierSurfaceMesh>
  create(const std::vector<float> &points, uint32_t u_patches,
         uint32_t v_patches);
  static std::unique_ptr<BezierSurfaceMesh>
  createC2(const std::vector<float> &points, uint32_t u_patches,
           uint32_t v_patches);

  ~BezierSurfaceMesh() override;

  BezierSurfaceMesh(BezierSurfaceMesh &&other) noexcept;
  BezierSurfaceMesh &operator=(BezierSurfaceMesh &&other) noexcept;

  BezierSurfaceMesh(const BezierSurfaceMesh &) = delete;
  BezierSurfaceMesh &operator=(const BezierSurfaceMesh &) = delete;

protected:
  explicit BezierSurfaceMesh(const std::vector<float> &vertices,
                             uint32_t u_patches, uint32_t v_patches);

private:
  uint32_t _vao, _vbo, _ebo; // these may need to be changed idk why?
  std::vector<float> _controlPoints;
  void initBuffers();

  static std::vector<float> createC0MeshData(const std::vector<float> &vertices,
                                             uint32_t u_patches,
                                             uint32_t v_patches);
};