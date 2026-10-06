cmake_minimum_required(VERSION 3.10)
project(RasterTemplate C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# По умолчанию собираем Release: программный растеризатор в Debug в разы медленнее
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


# --- утилиты шаблона: загрузка .obj и картинок ----------------------------
# Сюда же можно добавлять свои файлы (математика, растеризатор, ...)
add_library(raster_utils STATIC
    src/image_utils.c
    src/mesh_utils.c
)
target_include_directories(raster_utils PUBLIC src ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT MSVC)
    target_link_libraries(raster_utils PUBLIC m)
endif()

find_package(OpenMP)
if(OpenMP_C_FOUND)
    target_link_libraries(raster_utils PUBLIC OpenMP::OpenMP_C)
endif()

# --- консольная программа: меш + картинка -> PNG ---------------------------
add_executable(main_cli src/main_cli.c)
target_link_libraries(main_cli PRIVATE raster_utils)

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
    add_executable(main_app src/main_app.c)
    target_link_libraries(main_app PRIVATE raster_utils glfw OpenGL::GL)
else()
    message(WARNING "GLFW or OpenGL not found: main_app will not be built. "
                    "On Debian/Ubuntu run: sudo apt install libglfw3-dev")
endif()
