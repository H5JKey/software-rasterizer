cmake_minimum_required(VERSION 3.10)
project(SoftwareRaster CXX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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

add_library(raster
    src/image-utils.cpp
    src/mesh-loader.cpp
    src/vectors.cpp
)

target_include_directories(raster PUBLIC
    external
    include
)

# --- консольная программа: меш + картинка -> PNG ---------------------------

add_executable(main-cli src/main-cli.cpp)
target_link_libraries(main-cli PRIVATE raster)


# --- программа с окном (нужен GLFW) ---------------------------------------

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)
find_package(glfw3 3.3 QUIET)

if (NOT glfw3_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.5.1
    )

    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")

    FetchContent_MakeAvailable(glfw)

    set(glfw3_FOUND ON)
endif()

if(glfw3_FOUND AND OpenGL_FOUND)
    add_executable(main-app src/main-app.cpp)
    target_link_libraries(main-app PRIVATE 
        glfw 
        OpenGL::GL
        raster
    )
else()
    message(WARNING "GLFW or OpenGL not found: main-app will not be built. "
                    "On Debian/Ubuntu run: sudo apt install libglfw3-dev")
endif()