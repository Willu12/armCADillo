#pragma once
#include "IEntity.hpp"
#include "glad/gl.h"
#include <vector>

class IEntityRenderer {
public:
  virtual ~IEntityRenderer() = default;
  virtual void
  render(const std::vector<std::shared_ptr<IEntity>> &entities) = 0;

protected:
  GLuint prepareInstacedModelMatrices(
      const std::vector<std::shared_ptr<IEntity>> &entities) {
    std::vector<algebra::Mat4f> modelMatrices;
    for (const auto &entity : entities) {
      modelMatrices.push_back(entity->getModelMatrix().transpose());
    }

    GLuint modelBuffer;
    glGenBuffers(1, &modelBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(algebra::Mat4f),
                 modelMatrices.data(), GL_DYNAMIC_DRAW);

    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(1 + i);
      glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE,
                            sizeof(algebra::Mat4f),
                            (void *)(sizeof(float) * i * 4));
      glVertexAttribDivisor(1 + i, 1);
    }
    return modelBuffer;
  }
};
