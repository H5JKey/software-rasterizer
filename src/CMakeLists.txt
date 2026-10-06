add_subdirectory(utils)
add_subdirectory(math)
add_subdirectory(renderer)

add_library(raster INTERFACE)
target_link_libraries(raster INTERFACE
    raster_renderer
    raster_utils
    raster_math
)

# --- консольная программа: меш + картинка -> PNG ---------------------------

add_executable(main_cli main_cli.cpp)
target_link_libraries(main_cli PRIVATE raster)


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
    add_executable(main_app main_app.cpp)
    target_link_libraries(main_app PRIVATE 
        glfw 
        OpenGL::GL
        raster
    )
else()
    message(WARNING "GLFW or OpenGL not found: main_app will not be built. "
                    "On Debian/Ubuntu run: sudo apt install libglfw3-dev")
endif()