mspm0c110x_add_firmware(
    NAME ssd1306_hello_world
    SOURCES
        main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drv/ssd1306/ssd1306.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drv/ssd1306/ssd1306_printf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drv/ssd1306/ssd1306_font_8x8.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_gpio.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_i2c.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_common.c
    LINK_LIBRARIES
        xprintf
)

target_include_directories(ssd1306_hello_world PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drv/ssd1306
)
