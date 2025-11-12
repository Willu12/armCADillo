#pragma once

#include "glad/gl.h"
#include "matrix.hpp"
#include "vec.hpp"
#include <string>
#include <vector>

struct ShaderPath {
  std::string _path;
  int _type;
};

class Shader {
public:
  explicit Shader(const std::vector<ShaderPath> &shaderPaths);
  Shader(const std::string &vertexPath, const std::string &fragmentPath);
  Shader(const std::string &vertexPath, const std::string &geometryPath,
         const std::string &fragmentPath);

  void use() const;

  void setMat4f(const std::string &name, const algebra::Mat4f &mat) const;
  void setVec2f(const std::string &name, const algebra::Vec2f &value) const;
  void setVec3f(const std::string &name, const algebra::Vec3f &value) const;
  void setVec4f(const std::string &name, const algebra::Vec4f &value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setUInt(const std::string &name, uint32_t value) const;

  void setViewMatrix(const algebra::Mat4f &view) const;
  void setModelMatrix(const algebra::Mat4f &model) const;
  void setProjectionMatrix(const algebra::Mat4f &projection) const;

private:
  unsigned int _id{};
  static void checkCompileErrors(unsigned int shader, const std::string &type);
  static std::string readShaderFromFile(const std::string &shaderPath);
  static GLuint createShader(const std::string &shaderSource,
                             GLenum shaderType);
  static GLuint createProgram(const std::vector<GLuint> &shaderIds);
};