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

#include <GLFW/glfw3.h>
#include <stdio.h>

#define WIDTH 900
#define HEIGHT 900

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  ImGuiIO &io = ImGui::GetIO();
  io.MouseWheel += static_cast<float>(yoffset); // Update ImGui's mouse wheel
}

// Register the callback

int main(int, char **) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window = glfwCreateWindow(
      WIDTH, HEIGHT, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
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
  TorusSettings torusSettings(&torusModel);

  Shader shader("../shaders/vertexShader.hlsl",
                "../shaders/fragmentShader.hlsl");

  CameraController cameraController;
  MeshRenderer MeshRenderer(cameraController.getCamera(), &torusModel);
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

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    torusSettings._change |= (cameraController.processScroll());

    torusSettings.ShowSettingsWindow();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    if (cameraController.processMouse())
      torusSettings._change = true;

    if (torusSettings._change) {
      torusMesh = torusModel.generateMesh(shader);
      torusSettings._change = false;
    }
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