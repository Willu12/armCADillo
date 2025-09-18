#pragma once

#include "IMeshable.hpp"
#include "gregoryQuad.hpp"
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>

class GregoryMesh : public IMeshable {
public:
  uint32_t getVAO() const override;
  uint32_t getVBO() const override;
  uint32_t getEBO() const override;
  uint32_t getIndicesLength() const override;

  static std::unique_ptr<GregoryMesh> create(const GregoryQuad &quad);

  ~GregoryMesh() override;

  GregoryMesh(GregoryMesh &&other) noexcept;
  GregoryMesh &operator=(GregoryMesh &&other) noexcept;

  GregoryMesh(const GregoryMesh &) = delete;
  GregoryMesh &operator=(const GregoryMesh &) = delete;
  friend class GregorySurface;

protected:
  explicit GregoryMesh(const std::vector<float> &vertices);

private:
  uint32_t _vao, _vbo, _ebo; // these may need to be changed idk why?
  std::vector<float> _vertices;
  void initBuffers();

  static std::vector<float> createMeshData(const GregoryQuad &quad);
};