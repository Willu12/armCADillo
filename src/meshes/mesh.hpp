#pragma once
#include "IMeshable.hpp"
#include "IParametrizable.hpp"
#include <memory>
#include <vector>

struct MeshDensity {
  int s = 10;
  int t = 10;
};

class Mesh : public IMeshable {
public:
  uint32_t getVAO() const override;
  uint32_t getVBO() const override;
  uint32_t getEBO() const override;
  uint32_t getIndicesLength() const override;

  static std::unique_ptr<Mesh> create(const std::vector<float> &vertices,
                                      const std::vector<uint32_t> &indices);

  static std::unique_ptr<Mesh>
  createTexturedMesh(const std::vector<float> &vertices,
                     const std::vector<uint32_t> &indices);
  static std::unique_ptr<Mesh>
  fromParametrization(const algebra::IParametrizable<float> &parametrizable,
                      const MeshDensity &meshDensity);

  static std::unique_ptr<Mesh> fromParametrizationTextured(
      const algebra::IParametrizable<float> &parametrizable,
      const MeshDensity &meshDensity);

  ~Mesh();

  Mesh(Mesh &&other) noexcept;

  Mesh &operator=(Mesh &&other) noexcept;

  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

protected:
  Mesh(const std::vector<float> &vertices,
       const std::vector<uint32_t> &_indices);

private:
  uint32_t _vao, _vbo, _ebo;
  std::vector<float> _vertices;
  std::vector<uint32_t> _indices;

  void addSimpleVertexLayout();
  void addTextureLayout();
  void initBuffers();

  static std::vector<float>
  generateVertices(const algebra::IParametrizable<float> &parametrizable,
                   const MeshDensity &meshDensity);

  static std::vector<float> generateVerticesWithTexture(
      const algebra::IParametrizable<float> &parametrizable,
      const MeshDensity &meshDensity);

  static std::vector<unsigned int>
  generateIndices(const MeshDensity &meshDensitiy);
};