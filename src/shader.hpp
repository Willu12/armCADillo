#pragma once

#include "glad/glad.h"
#include "matrix.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class Shader {
public:
  void use() { glUseProgram(_id); }
  Shader(const std::string &vertexPath, const std::string &fragmentPath) {

    std::string vertexShaderSource = readShaderFromFile(vertexPath);
    std::string fragmentShaderSource = readShaderFromFile(fragmentPath);

    auto vertexShader = createShader(vertexShaderSource, GL_VERTEX_SHADER);
    auto fragmentShader =
        createShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    _id = createProgram({vertexShader, fragmentShader});
  }

  // TO DO: add SHADER PATH {string: path, SHADER_TYPE: tpye} struct
  Shader(const std::string &vertexPath, const std::string &geometryPath,
         const std::string &fragmentPath) {

    std::string vertexShaderSource = readShaderFromFile(vertexPath);
    std::string fragmentShaderSource = readShaderFromFile(fragmentPath);
    std::string geometryShaderSource = readShaderFromFile(geometryPath);

    auto vertexShader = createShader(vertexShaderSource, GL_VERTEX_SHADER);
    auto fragmentShader =
        createShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    auto geometryShader =
        createShader(geometryShaderSource, GL_GEOMETRY_SHADER);
    _id = createProgram({vertexShader, fragmentShader, geometryShader});
  }

  void setMat4f(const std::string &name, const algebra::Mat4f &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE,
                       &mat(0, 0));
  }

  void setVec3f(const std::string &name, const algebra::Vec3f &value) const {
    glUniform3fv(glGetUniformLocation(_id, name.c_str()), 1, &value[0]);
  }

  void setViewMatrix(const algebra::Mat4f &view) const {
    setMat4f("view", view.transpose());
  }

  void setModelMatrix(const algebra::Mat4f &model) const {
    setMat4f("model", model.transpose());
  }
  void setProjectionMatrix(const algebra::Mat4f &projection) const {
    setMat4f("projection", projection.transpose());
  }

  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
  }

  void setUInt(const std::string &name, uint32_t value) const {
    glUniform1ui(glGetUniformLocation(_id, name.c_str()), value);
  }

private:
  unsigned int _id;
  static void checkCompileErrors(unsigned int shader, const std::string &type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        throw std::runtime_error("ERROR::SHADER_COMPILATION_ERROR of type: " +
                                 type + "\n" + infoLog);
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        throw std::runtime_error(
            "ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog);
      }
    }
  }

  static std::string readShaderFromFile(const std::string &shaderPath) {
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
      shaderFile.open(shaderPath);

      std::stringstream shaderStream;
      shaderStream << shaderFile.rdbuf();
      shaderFile.close();
      shaderCode = shaderStream.str();
    } catch (std::ifstream::failure &e) {
      std::string errorMessage =
          "ERROR: Shader file could not be opened or read from path: " +
          shaderPath;
      errorMessage += "\nException: ";
      throw std::runtime_error(errorMessage + e.what());
    }
    return shaderCode;
  }

  static GLuint createShader(const std::string &shaderSource,
                             GLenum shaderType) {
    GLuint shader;
    int success;

    shader = glCreateShader(shaderType);

    const char *shaderSourceCStr = shaderSource.c_str();
    glShaderSource(shader, 1, &shaderSourceCStr, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
      GLint logLength;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

      std::string infoLog(logLength, ' ');
      glGetShaderInfoLog(shader, logLength, &logLength, &infoLog[0]);

      throw std::runtime_error("ERROR::SHADER::" + std::to_string(shaderType) +
                               "::COMPILATION_FAILED\n" + infoLog);
    }

    return shader;
  }

  static GLuint createProgram(const std::vector<GLuint> &shaderIds) {
    auto id = glCreateProgram();
    for (auto shaderId : shaderIds) {
      glAttachShader(id, shaderId);
    }
    glLinkProgram(id);

    int success;
    char infoLog[512];
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(id, 512, NULL, infoLog);
      throw std::runtime_error("ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" +
                               std::string(infoLog) + '\n');
    }

    for (auto shaderId : shaderIds) {
      glDeleteShader(shaderId);
    }
    return id;
  }
};