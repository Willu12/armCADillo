#pragma once

#include <cstdint>
class IMeshable {
public:
  virtual ~IMeshable() = default;
  virtual uint32_t getVAO() const = 0;
  virtual uint32_t getVBO() const = 0;
  virtual uint32_t getEBO() const = 0;
  virtual uint32_t getIndicesLength() const = 0;
};