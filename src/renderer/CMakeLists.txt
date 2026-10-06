add_library(raster_renderer
    tmp.cpp
)

target_link_libraries(raster_renderer PUBLIC
    raster_math
    raster_utils
)

target_include_directories(raster_renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)