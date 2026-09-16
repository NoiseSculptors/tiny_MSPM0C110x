cmake_minimum_required(VERSION 3.20)

project(tiny_MSPM0C110x C)

set(MSPM0C110X_SDK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk/source")
set(MSPM0C110X_SDK_INCLUDE_DIR "${MSPM0C110X_SDK_SOURCE_DIR}")
set(MSPM0C110X_CMSIS_INCLUDE_DIR
    "${MSPM0C110X_SDK_SOURCE_DIR}/third_party/CMSIS/Core/Include")
set(MSPM0_DEVICE "MSPM0C1103" CACHE STRING
    "Target MSPM0C110x device")
set_property(CACHE MSPM0_DEVICE PROPERTY STRINGS
    MSPM0C1103 MSPM0C1104 MSPM0C1105 MSPM0C1106)

if(NOT MSPM0_DEVICE MATCHES "^MSPM0C110[3456]$")
    message(FATAL_ERROR
        "MSPM0_DEVICE must be one of MSPM0C1103, MSPM0C1104, MSPM0C1105, or MSPM0C1106")
endif()

string(TOLOWER "${MSPM0_DEVICE}" MSPM0_DEVICE_LOWER)
set(MSPM0C110X_LINKER_SCRIPT
    "${MSPM0C110X_SDK_SOURCE_DIR}/ti/devices/msp/m0p/linker_files/gcc/${MSPM0_DEVICE_LOWER}.lds")

add_library(mspm0c110x_build_options INTERFACE)
target_compile_options(mspm0c110x_build_options INTERFACE
    -mcpu=cortex-m0plus
    -mthumb
    -mfloat-abi=soft
    -ffunction-sections
    -fdata-sections
    -fno-common
    -Wall
    -Wextra
    -Os
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/mspm0c110x.cmake)

add_subdirectory(lib)
add_subdirectory(drv)
add_subdirectory(examples)
