add_library(raster_utils
    image_utils.c
    mesh_utils.cpp
)

target_link_libraries(raster_utils PRIVATE raster_math)

target_include_directories(raster_utils
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
    ${PROJECT_SOURCE_DIR}/external
)