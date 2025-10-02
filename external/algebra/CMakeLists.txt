set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(algebra INTERFACE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_include_directories(algebra INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/geometry
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rotation
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parametricForms
    ${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/include/distributions
)