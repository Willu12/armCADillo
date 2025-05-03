#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gui.hpp"

#include "sceneRenderer.hpp"

#include "camera.hpp"
#include <memory>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel += static_cast<float>(yoffset); // Update ImGui's mouse wheel
}

void setupViewPortAndClear(GLFWwindow *window, const ImVec4 &clearColor) {
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
               clearColor.z * clearColor.w, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
}

int main(int, char **) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char *glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "armCADillo", nullptr, nullptr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    return 1;
  }
  glfwSetScrollCallback(window, scrollCallback);

  std::shared_ptr<Camera> camera = std::make_shared<Camera>(window);
  std::shared_ptr<Scene> scene = std::make_shared<Scene>(camera);

  GUI gui(window, scene);

  SceneRenderer sceneRenderer(camera, gui.getPickingTexture(), window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImVec4 clear_color = ImVec4(0.1, 0.1f, 0.1f, 1.00f);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    gui.displayGUI();
    // Rendering
    ImGui::Render();

    setupViewPortAndClear(window, clear_color);

    sceneRenderer.render(scene->getGroupedEntities());

    if (gui.getCenterPoint())
      sceneRenderer.renderCenterPoint(*gui.getCenterPoint().value());

    if (gui.getMouse().anyButtonDown() && !scene->getPoints().empty())
      sceneRenderer.renderPicking(scene->getPoints());

    sceneRenderer.renderCursor(gui.getCursor());
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}