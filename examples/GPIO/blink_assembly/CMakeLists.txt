mspm0c110x_add_firmware(
    NAME blink_assembly
    SOURCES main.S
    NO_STARTUP
)

target_link_options(blink_assembly PRIVATE -Wl,-e,Reset_Handler)
