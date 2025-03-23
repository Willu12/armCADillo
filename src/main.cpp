#include "algebra.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

#include "cameraController.hpp"
#include "renderer.hpp"
#include "torusSettings.hpp"

#include "modelController.hpp"
#include "torusModel.hpp"

#include "grid.hpp"
#include <GLFW/glfw3.h>
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

int main(int, char **) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char *glsl_version = "#version 130";
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

  TorusModel torusModel(2.f, 1.0f);

  Shader shader("../shaders/vertexShader.hlsl",
                "../shaders/fragmentShader.hlsl");

  ModelController torusController(&torusModel);
  CameraController cameraController;
  TorusSettings torusSettings(&torusModel, &torusController);

  Grid grid(window);

  MeshRenderer MeshRenderer(cameraController.getCamera(), &torusModel, window);
  auto torusMesh = torusModel.generateMesh(shader);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
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

    if (torusSettings._controllerKind == TorusSettings::ControllerKind::Camera)
      cameraController.processScroll();
    else
      torusController.processScroll();

    torusSettings.ShowSettingsWindow();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    if (torusSettings._controllerKind == TorusSettings::ControllerKind::Camera)
      cameraController.processMouse();
    else
      torusController.processMouse();

    if (torusSettings._change) {
      torusMesh = torusModel.generateMesh(shader);
      torusSettings._change = false;
    }
    grid.render(cameraController.getCamera());

    MeshRenderer.renderMesh(torusMesh, shader);

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