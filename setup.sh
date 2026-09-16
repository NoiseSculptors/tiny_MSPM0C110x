#!/bin/sh
set -eu

sdk_url=https://github.com/TexasInstruments/mspm0-sdk
font8x8_url=https://github.com/dhepper/font8x8

if ! command -v git >/dev/null 2>&1; then
    printf '%s\n' 'Error: git is required to run setup.sh.' >&2
    exit 1
fi

if ! project_dir=$(git rev-parse --show-toplevel 2>/dev/null); then
    printf '%s\n' 'Error: setup.sh must be run from inside a Git repository.' >&2
    exit 1
fi

cd "$project_dir"

sdk_commit=20807db79aa17b49f87ab8ec87f6b6d63ee2cb32

configured_sdk_url=$(git config -f .gitmodules --get submodule.sdk.url 2>/dev/null || true)
if [ -z "$configured_sdk_url" ]; then
    if [ -e sdk ] || [ -L sdk ]; then
        printf '%s\n' 'Error: sdk/ exists but is not configured as the MSPM0 SDK submodule.' >&2
        printf '%s\n' 'Remove sdk/ and run setup.sh again.' >&2
        exit 1
    fi
    git clone --depth 1 --filter=blob:none --no-checkout "$sdk_url" sdk
    git config -f .gitmodules submodule.sdk.path sdk
    git config -f .gitmodules submodule.sdk.url "$sdk_url"
    git add .gitmodules sdk
    git submodule absorbgitdirs -- sdk
    sdk_added=1
elif [ "$configured_sdk_url" != "$sdk_url" ]; then
    printf 'Error: submodule sdk has unexpected URL: %s\n' "$configured_sdk_url" >&2
    printf 'Expected: %s\n' "$sdk_url" >&2
    exit 1
else
    sdk_added=0
fi

git submodule sync -- sdk

if [ "$sdk_added" -eq 0 ]; then
    git submodule update --init --depth 1 --filter=blob:none sdk
fi

git -C sdk fetch --depth 1 origin "$sdk_commit"

git -C sdk sparse-checkout init --no-cone
git -C sdk sparse-checkout set --no-cone \
    /source/ti/devices/DeviceFamily.h \
    /source/ti/devices/msp/m0p/linker_files/gcc/mspm0c1103.lds \
    /source/ti/devices/msp/m0p/linker_files/gcc/mspm0c1104.lds \
    /source/ti/devices/msp/m0p/linker_files/gcc/mspm0c1105.lds \
    /source/ti/devices/msp/m0p/linker_files/gcc/mspm0c1106.lds \
    /source/ti/devices/msp/m0p/mspm0c110x.h \
    /source/ti/devices/msp/m0p/startup_system_files/gcc/startup_mspm0c110x_gcc.c \
    /source/ti/devices/msp/msp.h \
    /source/ti/devices/msp/peripherals/hw_adc12.h \
    /source/ti/devices/msp/peripherals/hw_crc.h \
    /source/ti/devices/msp/peripherals/hw_dma.h \
    /source/ti/devices/msp/peripherals/hw_flashctl.h \
    /source/ti/devices/msp/peripherals/hw_gpio.h \
    /source/ti/devices/msp/peripherals/hw_gptimer.h \
    /source/ti/devices/msp/peripherals/hw_i2c.h \
    /source/ti/devices/msp/peripherals/hw_iomux.h \
    /source/ti/devices/msp/peripherals/hw_spi.h \
    /source/ti/devices/msp/peripherals/hw_uart.h \
    /source/ti/devices/msp/peripherals/hw_vref.h \
    /source/ti/devices/msp/peripherals/hw_wuc.h \
    /source/ti/devices/msp/peripherals/hw_wwdt.h \
    /source/ti/devices/msp/peripherals/m0p/hw_cpuss.h \
    /source/ti/devices/msp/peripherals/m0p/hw_debugss.h \
    /source/ti/devices/msp/peripherals/m0p/hw_factoryregion.h \
    /source/ti/devices/msp/peripherals/m0p/hw_sysctl.h \
    /source/ti/devices/msp/peripherals/m0p/sysctl/hw_sysctl_mspm0c110x.h \
    /source/ti/driverlib/dl_common.c \
    /source/ti/driverlib/dl_common.h \
    /source/ti/driverlib/dl_gpio.c \
    /source/ti/driverlib/dl_gpio.h \
    /source/ti/driverlib/dl_i2c.c \
    /source/ti/driverlib/dl_i2c.h \
    /source/ti/driverlib/dl_spi.c \
    /source/ti/driverlib/dl_spi.h \
    /source/ti/driverlib/dl_timer.c \
    /source/ti/driverlib/dl_timer.h \
    /source/ti/driverlib/dl_timera.h \
    /source/ti/driverlib/dl_timerg.h \
    /source/ti/driverlib/dl_uart.c \
    /source/ti/driverlib/dl_uart.h \
    /source/ti/driverlib/dl_uart_extend.h \
    /source/ti/driverlib/dl_uart_main.h \
    /source/ti/driverlib/m0p/dl_core.h \
    /source/ti/driverlib/m0p/dl_factoryregion.h \
    /source/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c1105_c1106.c \
    /source/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c1105_c1106.h \
    /source/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.c \
    /source/ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h \
    /source/third_party/CMSIS/Core/Include/core_cm0plus.h \
    /source/third_party/CMSIS/Core/Include/cmsis_compiler.h \
    /source/third_party/CMSIS/Core/Include/cmsis_gcc.h \
    /source/third_party/CMSIS/Core/Include/cmsis_version.h \
    /source/third_party/CMSIS/Core/Include/mpu_armv7.h

git -C sdk checkout --detach "$sdk_commit"

configured_font8x8_url=$(git config -f .gitmodules --get submodule.lib/font8x8.url 2>/dev/null || true)
if [ -z "$configured_font8x8_url" ]; then
    if [ -e lib/font8x8 ] || [ -L lib/font8x8 ]; then
        printf '%s\n' 'Error: lib/font8x8 exists but is not configured as the font8x8 submodule.' >&2
        printf '%s\n' 'Remove lib/font8x8 and run setup.sh again.' >&2
        exit 1
    fi
    git submodule add "$font8x8_url" lib/font8x8
elif [ "$configured_font8x8_url" != "$font8x8_url" ]; then
    printf 'Error: submodule lib/font8x8 has unexpected URL: %s\n' "$configured_font8x8_url" >&2
    printf 'Expected: %s\n' "$font8x8_url" >&2
    exit 1
fi

font8x8_entry=$(git ls-files --stage -- lib/font8x8)
case "$font8x8_entry" in
    "160000 "*) ;;
    *)
        printf '%s\n' 'Error: lib/font8x8 is not recorded as a Git submodule.' >&2
        exit 1
        ;;
esac

git submodule sync -- lib/font8x8
git submodule update --init --depth 1 -- lib/font8x8
