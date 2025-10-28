#pragma once

#include "scene.hpp"
#include "sceneRenderer.hpp"

#include "camera.hpp"
#include "gui.hpp"
#include <memory>

class App {
public:
  void run();

private:
  GLFWwindow *window_ = nullptr;
  std::unique_ptr<Camera> camera_;
  std::unique_ptr<Scene> scene_;
  std::unique_ptr<GUI> gui_;
  std::unique_ptr<SceneRenderer> sceneRenderer_;

  void init();
  void initImgui();
  void mainLoop();
  void cleanUp();

  static void glfw_error_callback(int error, const char *description);
  static void scrollCallback(GLFWwindow * /*window*/, double /*xoffset*/,
                             double yoffset);
  static void setupViewPortAndClear(GLFWwindow *window,
                                    const ImVec4 &clearColor);
};