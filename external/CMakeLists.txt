add_library(imgui
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/imgui_tables.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
)

add_library(stb_image INTERFACE)

target_include_directories(stb_image INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image
)

add_subdirectory(glfw)
add_subdirectory(algebra)
add_subdirectory(glad)

target_include_directories(imgui PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/imgui
   ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
   ${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp
)

target_link_libraries(imgui PUBLIC glfw)
