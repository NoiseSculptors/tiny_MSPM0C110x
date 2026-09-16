add_library(st7735s STATIC
    st7735s.c
    st7735s_conf.c
    st7735s_printf.c
)

target_include_directories(st7735s PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/font8x8
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/source
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/source/third_party/CMSIS/Core/Include
)

target_compile_definitions(st7735s PRIVATE DeviceFamily_MSPM0C110X)
target_link_libraries(st7735s PUBLIC xprintf)
target_link_libraries(st7735s PUBLIC mspm0c110x_build_options)

target_compile_features(st7735s PUBLIC c_std_99)
