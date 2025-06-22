cmake_minimum_required(VERSION 3.22)
project(armCADillo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(CMAKE_CXX_FLAGS "-fsanitize=address,undefined -g")
#set(CMAKE_LINKER_FLAGS "-fsanitize=address,undefined")

add_subdirectory(external)

add_executable(${PROJECT_NAME}
 src/main.cpp
 src/notifications/ISubsriber.cpp
 src/gui/gui.cpp
 src/gui/visitors/GuiVisitor.cpp
 src/gui/visitors/jsonSerializer.cpp
 src/entities/virtualPoint.cpp
 src/entities/gregorySurface.cpp
 src/entities/polyline.cpp
 src/entities/interpolatingSplineC2.cpp
 src/controllers/selectionController.cpp
 src/entities/bezierSurfaceC0.cpp
 src/entities/bezierSurfaceC2.cpp
 src/utils/json/entityDeserializer.cpp
 src/utils/json/torusDeserializer.cpp
 src/utils/borderGraph.cpp
 src/utils/json/bezierCurveC0Deseralizer.cpp
 src/scene.cpp
 src/intersections/intersectionFinder.cpp

 )

target_include_directories(${PROJECT_NAME} PRIVATE
    src/include/
    src/gui/
    src/textures/
    src/controllers/
    src/entities/
    src/utils/
    src/utils/json
    src/
    src/rendering/
    src/meshes/
    src/renderables/
    src/rendering/EntityRenderers/
    src/notifications/
    src/gui/visitors/
    src/intersections
)

target_link_libraries(${PROJECT_NAME} imgui algebra glad glfw GL dl stb_image nlohmann_json::nlohmann_json nfd)
