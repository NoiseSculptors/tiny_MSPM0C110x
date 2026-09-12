include(CMakeParseArguments)

function(mspm0c110x_add_firmware)
    set(options)
    set(one_value_args NAME)
    set(multi_value_args SOURCES LINK_LIBRARIES)
    cmake_parse_arguments(FW "${options}" "${one_value_args}"
        "${multi_value_args}" ${ARGN})

    if(NOT FW_NAME)
        message(FATAL_ERROR "mspm0c110x_add_firmware requires NAME")
    endif()

    add_executable(${FW_NAME}
        ${FW_SOURCES}
        ${MSPM0C110X_SDK_SOURCE_DIR}/ti/devices/msp/m0p/startup_system_files/gcc/startup_mspm0c110x_gcc.c
    )

    target_compile_definitions(${FW_NAME} PRIVATE
        DeviceFamily_MSPM0C110X
    )

    target_include_directories(${FW_NAME} PRIVATE
        ${MSPM0C110X_SDK_INCLUDE_DIR}
        ${MSPM0C110X_CMSIS_INCLUDE_DIR}
    )

    target_link_libraries(${FW_NAME} PRIVATE
        mspm0c110x_build_options
        ${FW_LINK_LIBRARIES}
    )

    target_link_options(${FW_NAME} PRIVATE
        -mcpu=cortex-m0plus
        -mthumb
        -mfloat-abi=soft
        -nostartfiles
        --specs=nosys.specs
        -T${MSPM0C110X_LINKER_SCRIPT}
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.map
    )

    set_target_properties(${FW_NAME} PROPERTIES SUFFIX ".elf")

    add_custom_command(TARGET ${FW_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${FW_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:${FW_NAME}>
            ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.hex
        VERBATIM
    )
endfunction()
