cmake_minimum_required(VERSION 3.22)
project(armCADillo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external)

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    src/backend/include/
    src/frontend/include/
    src/frontend/include/gui/
    src/frontend/include/textures/
    src/frontend/include/controllers/
    src/backend/include/entities/

)

target_link_libraries(${PROJECT_NAME} imgui algebra glad glfw GL dl stb_image)
