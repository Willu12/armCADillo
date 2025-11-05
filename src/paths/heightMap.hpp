#pragma once

#include "block.hpp"
#include "cutter.hpp"
#include "texture.hpp"
#include "vec.hpp"
#include <cstdint>
#include <vector>

struct Divisions {
  uint32_t x_ = 1500;
  uint32_t z_ = 1500;
};

class HeightMapGenerator;

class HeightMap {
public:
  HeightMap(Divisions divisions, float baseHeight, const Block *block)
      : divisions_(divisions), baseHeight_(baseHeight), block_(block) {}

  const Divisions &divisions() const { return divisions_; }
  float baseHeight() const { return baseHeight_; }
  float &at(uint32_t index) { return data_[index]; }
  float at(uint32_t index) const { return data_[index]; }
  const Block &block() const { return *block_; }

  algebra::Vec3f &normalAtIndex(uint32_t index);
  const algebra::Vec3f &normalAtIndex(uint32_t index) const;

  float findMinimumSafeHeightForCut(
      uint32_t index, const Cutter &cutter) const; // this should be moved

  uint32_t textureId() const { return texture_->getTextureId(); }

  friend class HeightMapGenerator;
  friend class PathsGenerator;
  friend class RoughingPathGenerator;
  friend class FlatPathGenerator;

private:
  Divisions divisions_;
  float baseHeight_;
  const Block *block_;
  std::unique_ptr<Texture> texture_ =
      Texture::createTexture(divisions_.x_, divisions_.z_);

  std::vector<float> data_ =
      std::vector<float>(divisions_.x_ * divisions_.z_, baseHeight_);

  std::vector<algebra::Vec3f> normalData_ =
      std::vector<algebra::Vec3f>(divisions_.x_ * divisions_.z_);

  std::pair<uint32_t, uint32_t> pixelCmRatio() const;
  algebra::Vec3f indexToPos(uint32_t index) const;
  uint32_t globalIndex(uint32_t x, uint32_t z) const;
  uint32_t posToIndex(const algebra::Vec3f &position) const;
  void updateTexture();
};