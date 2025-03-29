#include "algebra.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>

#include "cameraController.hpp"
#include "gui.hpp"
#include "renderer.hpp"

#include "cursor.hpp"
#include "cursorController.hpp"
#include "modelController.hpp"
#include "point.hpp"
#include "torusModel.hpp"

#include "image.hpp"
#include "texture.hpp"
#include "textureResource.hpp"

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

  // TEXTURE
  Image image("../resources/textures/cursorTexture.png");
  TextureResource textureResource(image);
  Texture texture(textureResource);

  Shader shader("../resources/shaders/vertexShader.hlsl",
                "../resources/shaders/fragmentShader.hlsl");

  Shader textureShader("../resources/shaders/textureShader.vert",
                       "../resources/shaders/texturedBillboardShader.frag");

  Camera *camera = new Camera(window);

  GUI gui(window, camera);
  Grid grid(window);

  MeshRenderer MeshRenderer(camera, window);

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

    gui.displayGUI();
    gui.getController().processScroll();

    // Rendering
    ImGui::Render();
    setupViewPortAndClear(window, clear_color);

    gui.getController().processMouse();
    grid.render(camera);
    MeshRenderer.renderEntities(gui.getEntities(), shader);
    MeshRenderer.renderBillboard(gui.getCursor(), textureShader);

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