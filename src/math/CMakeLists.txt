add_library(raster_math
    vectors.cpp
)

target_include_directories(raster_math PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)