#include "glad/gl.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "app.hpp"
#include "appConfig.hpp"
#include "imgui.h"

#include "sceneRenderer.hpp"
#include <cstdlib>
#include <memory>
#include <print>
#include <stdexcept>

void App::run() {
  init();
  mainLoop();
  cleanUp();
}

void App::init() {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("failed to init GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  window_ = glfwCreateWindow(AppConfig::kWidth, AppConfig::kHeight,
                             AppConfig::kName, nullptr, nullptr);
  if (window_ == nullptr) {
    throw std::runtime_error("failed to init window");
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1); // Enable vsync

  if (!gladLoaderLoadGL()) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glfwSetScrollCallback(window_, scrollCallback);

  camera_ = std::make_unique<Camera>(window_);
  scene_ = std::make_unique<Scene>(camera_.get());
  gui_ = std::make_unique<GUI>(window_, scene_.get());
  sceneRenderer_ = std::make_unique<SceneRenderer>(
      camera_.get(), gui_->getPickingTexture(), window_);

  initImgui();
}

void App::initImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init(AppConfig::kGlslVersion);
}

void App::mainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window_, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    gui_->displayGUI();

    ImGui::Render();
    setupViewPortAndClear(window_, AppConfig::kClearColor);

    scene_->processFrame();

    if (gui_->stereographicVision()) {
      sceneRenderer_->stereoscopicRender(scene_->getGroupedEntities());
    } else {
      sceneRenderer_->render(scene_->getGroupedEntities());
    }

    sceneRenderer_->renderSelectedPoints(
        gui_->getSelectedPointsPointers()); // NOT EFFICIENT
    if (gui_->getCenterPoint()) {
      sceneRenderer_->renderCenterPoint(*gui_->getCenterPoint().value());
    }

    if (gui_->getMouse().leftButtonDown() && !scene_->getPickables().empty()) {
      sceneRenderer_->renderPicking(scene_->getPickables());
    }

    sceneRenderer_->renderSelectionBox(gui_->getMouse());
    sceneRenderer_->renderCursor(gui_->getCursor());
    sceneRenderer_->renderVirtualPoints(gui_->getVirtualPoints());
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window_);
  }
}

void App::cleanUp() {
  scene_.reset();
  gui_.reset();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window_);
  glfwTerminate();
}
void App::glfw_error_callback(int error, const char *description) {
  std::println(stderr, "GLFW Error {}: {}", error, description);
}

void App::scrollCallback(GLFWwindow * /*window*/, double /*xoffset*/,
                         double yoffset) {
  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel += static_cast<float>(yoffset);
}

void App::setupViewPortAndClear(GLFWwindow *window, const ImVec4 &clearColor) {
  int displayW = 0;
  int displayH = 0;
  glfwGetFramebufferSize(window, &displayW, &displayH);
  glViewport(0, 0, displayW, displayH);
  glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
               clearColor.z * clearColor.w, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
}