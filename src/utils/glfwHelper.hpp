#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

namespace GLFWHelper {
static float getAspectRatio(GLFWwindow *window) {
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  return static_cast<float>(width) / static_cast<float>(height);
}

static int getWidth(GLFWwindow *window) {
  int width = 0;
  glfwGetFramebufferSize(window, &width, nullptr);
  return width;
}

static int getHeight(GLFWwindow *window) {
  int height = 0;
  glfwGetFramebufferSize(window, nullptr, &height);
  return height;
}
} // namespace GLFWHelper