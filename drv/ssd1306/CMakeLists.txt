add_library(ssd1306 STATIC
    ssd1306.c
    ssd1306_conf.c
    ssd1306_font_8x8.c
    ssd1306_printf.c
)

target_include_directories(ssd1306 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/source
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/source/third_party/CMSIS/Core/Include
)

target_compile_definitions(ssd1306 PRIVATE DeviceFamily_MSPM0C110X)
target_link_libraries(ssd1306 PUBLIC xprintf)
target_link_libraries(ssd1306 PUBLIC mspm0c110x_build_options)
target_compile_features(ssd1306 PUBLIC c_std_99)
