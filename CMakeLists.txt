cmake_minimum_required(VERSION 3.28) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(SoftwareRaster CXX C)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        add_compile_options(/Wall /wd4505 /wd5045 /wd4820 /wd4242 /wd4244 /wd4996 /wd4668 /wd4100 /Od /Zi /DEBUG)
    else()
        add_compile_options(-Wall -Wno-unused-function -O0 -g)
    endif()
else()
    if(MSVC)
        add_compile_options(/wd4505 /wd5045 /wd4820 /wd4242 /wd4244 /wd4996 /wd4668 /wd4100 /O2)
    else()
        add_compile_options(-Wall -Wno-unused-function -O3 -march=native)
    endif()
endif()

if (WIN32)
    add_compile_definitions(NOMINMAX)
endif()

add_executable(raster
    src/utils/image-utils.cpp
    src/renderer/rasterizer.cpp
    src/model/texture.cpp
    src/model/mesh.cpp
    src/utils/stb-impl.cpp
    src/utils/tinyobj-impl.cpp
    src/model/object.cpp
    src/app/app.cpp
    src/main.cpp
    src/scene/camera.cpp
)

target_sources(raster
PUBLIC
    FILE_SET CXX_MODULES
    FILES
        src/math/matrices.cppm
        src/math/vectors.cppm
        src/math/transforms.cppm
        src/math/math.cppm
        src/renderer/rasterizer.cppm
        src/renderer/shaders.cppm
        src/scene/camera.cppm
        src/renderer/framebuffer.cppm
        src/model/material.cppm
        src/model/mesh.cppm
        src/model/object.cppm
        src/scene/scene.cppm
        src/model/texture.cppm
        src/utils/image-utils.cppm
        src/app/app.cppm
        src/app/theme.cppm

)

target_include_directories(raster PUBLIC
    external
)

find_package(OpenMP QUIET)
if (OpenMP_FOUND)
    target_link_libraries(raster PUBLIC  OpenMP::OpenMP_CXX)
endif()

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.5.1
    FIND_PACKAGE_ARGS NAMES glfw 
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.93.0
)


set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(glfw imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw)

target_link_libraries(raster PRIVATE 
    glfw 
    OpenGL::GL
    imgui
)