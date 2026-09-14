add_subdirectory(xprintf)

add_library(petit_fatfs STATIC
    petit_fatfs/pff.c
)

target_link_libraries(petit_fatfs PRIVATE
    mspm0c110x_build_options
)

target_include_directories(petit_fatfs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/petit_fatfs
    ${CMAKE_CURRENT_SOURCE_DIR}/../examples/GPIO/petit_fatfs_bitbang
)
