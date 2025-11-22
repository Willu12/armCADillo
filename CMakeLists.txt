cmake_minimum_required(VERSION 3.22)
project(armCADillo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(CMAKE_CXX_FLAGS "-fsanitize=address,undefined -g")
#set(CMAKE_LINKER_FLAGS "-fsanitize=address,undefined")

add_subdirectory(external)

add_executable(${PROJECT_NAME}
 src/app/app.cpp
 src/controllers/selectionController.cpp
 src/entities/IEntity.cpp
 src/entities/bezierSurface.cpp
 src/entities/bezierSurfaceC0.cpp
 src/entities/bezierSurfaceC2.cpp
 src/entities/bSplineCurve.cpp
 src/entities/entityFactory/entityFactory.cpp
 src/entities/gregorySurface.cpp
 src/entities/interpolatingSplineC2.cpp
 src/entities/intersectable.cpp
 src/entities/intersectionCurve.cpp
 src/entities/polyline.cpp
 src/entities/virtualPoint.cpp
 src/gui/entityBuilders/entityBuilder.cpp
 src/gui/gui.cpp
 src/gui/pathCombinerGui.cpp
 src/gui/visitors/GuiVisitor.cpp
 src/gui/visitors/jsonSerializer.cpp
 src/intersections/intersectionFinder.cpp
 src/main.cpp
 src/meshes/bezierCurveMesh.cpp
 src/meshes/bezierSurfaceMesh.cpp
 src/meshes/gregoryMesh.cpp
 src/meshes/mesh.cpp
 src/notifications/ISubsriber.cpp
 src/paths/detailedPathGenerator.cpp
 src/paths/flatPathGenerator.cpp
 src/paths/GCodeSerializer.cpp
 src/paths/heightMap.cpp
 src/paths/heightMapGenerator.cpp
 src/paths/pathCombiner.cpp
 src/paths/pathsGenerator.cpp
 src/paths/pathReader.cpp
 src/paths/roughingPathGenerator.cpp
 src/paths/namedPath.cpp
 src/rendering/millingPathRenderer.cpp
 src/scene.cpp
 src/shader.cpp
 src/textures/image.cpp
 src/textures/intersectionTexture.cpp
 src/utils/borderGraph.cpp
 src/utils/json/bezierCurveC0Deseralizer.cpp
 src/utils/json/entityDeserializer.cpp
 src/utils/json/torusDeserializer.cpp
 )

target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    src/app
    src/controllers/
    src/entities/
    src/entities/entityFactory
    src/gui/
    src/gui/visitors/
    src/include/
    src/intersections
    src/meshes/
    src/notifications/
    src/paths
    src/renderables/
    src/rendering/
    src/rendering/EntityRenderers/
    src/textures/
    src/utils/
    src/utils/json
)

target_link_libraries(${PROJECT_NAME} imgui algebra glad glfw GL dl stb_image nlohmann_json::nlohmann_json nfd)
