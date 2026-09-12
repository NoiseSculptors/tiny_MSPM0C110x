add_library(xprintf STATIC
    src/xprintf.c
)

target_link_libraries(xprintf PRIVATE
    mspm0c110x_build_options
)

target_include_directories(xprintf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
