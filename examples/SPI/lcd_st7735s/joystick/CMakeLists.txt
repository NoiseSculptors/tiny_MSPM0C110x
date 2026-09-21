mspm0c110x_add_firmware(
    NAME st7735s_joystick
    SOURCES
        main.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_adc12.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_gpio.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_spi.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_timer.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_common.c
    LINK_LIBRARIES
        st7735s
)
