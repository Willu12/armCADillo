#pragma once

#include "GLFW/glfw3.h"

namespace GLFWHelper {
static float getAspectRatio(GLFWwindow *window) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  return static_cast<float>(width) / static_cast<float>(height);
}

static int getWidth(GLFWwindow *window) {
  int width;
  glfwGetFramebufferSize(window, &width, nullptr);
  return width;
}

static int getHeight(GLFWwindow *window) {
  int height;
  glfwGetFramebufferSize(window, nullptr, &height);
  return height;
}
} // namespace GLFWHelper