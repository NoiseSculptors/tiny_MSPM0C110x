mspm0c110x_add_firmware(
    NAME ssd1306_hello_world
    SOURCES
        main.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_gpio.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_i2c.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_timer.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_common.c
    LINK_LIBRARIES
        ssd1306
)

target_include_directories(ssd1306_hello_world PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drv/ssd1306
)
