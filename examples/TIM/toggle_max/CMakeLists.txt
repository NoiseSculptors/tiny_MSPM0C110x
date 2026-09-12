mspm0c110x_add_firmware(
    NAME toggle_max
    SOURCES
        main.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_common.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_gpio.c
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/driverlib/dl_timer.c
)
