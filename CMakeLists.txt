cmake_minimum_required(VERSION 3.22)
project(armCADillo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer")


add_subdirectory(external)

add_executable(${PROJECT_NAME} src/main.cpp src/rendering/renderer.cpp src/notifications/ISubsriber.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    src/include/
    src/gui/
    src/textures/
    src/controllers/
    src/entities/
    src/utils/
    src/
    src/rendering/
    src/meshes/
    src/renderables/
    src/rendering/EntityRenderers/
    src/notifications/

)

target_link_libraries(${PROJECT_NAME} imgui algebra glad glfw GL dl stb_image)
