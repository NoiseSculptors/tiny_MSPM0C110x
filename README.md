# tiny_MSPM0C110x

Bare-metal CMake project and examples for the Texas Instruments MSPM0C110x
family, using `arm-none-eabi-gcc` and the parts of the official TI MSPM0 SDK.

## Requirements

- `git`
- `cmake` 3.20 or newer
- `arm-none-eabi-gcc` and its binutils

## Setup And Build

From this directory:

```sh
./setup.sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build
```
The default target device is `MSPM0C1103`. Select another device when
configuring with `MSPM0_DEVICE`:

```sh
cmake -S . -B build-1104 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \
  -DMSPM0_DEVICE=MSPM0C1104
cmake --build build-1104
```
Valid values are `MSPM0C1103`, `MSPM0C1104`, `MSPM0C1105`, and
`MSPM0C1106`. When reusing an existing build directory, rerun the configure
command with the desired `MSPM0_DEVICE` before building.

Each firmware example produces its own `.elf`, `.bin`, `.hex`, and `.map` in
its build directory.

## Adding Examples And Libraries

Examples live under `examples/`. Add a directory with a `CMakeLists.txt` and
`main.c`, then register it in `examples/CMakeLists.txt`:

```cmake
mspm0c110x_add_firmware(
    NAME my_example
    SOURCES main.c
    LINK_LIBRARIES xprintf
)
```
Add reusable libraries under `lib/`, give each one a CMake target, and register
it in `lib/CMakeLists.txt`. Libraries should publish their own include paths;
firmwares should consume them through `LINK_LIBRARIES`.

## Updating the SDK

The project is pinned to SDK commit
`20807db79aa17b49f87ab8ec87f6b6d63ee2cb32` from release
`mspm0_sdk_2_11_00_07`. To update it, change `sdk_commit` in `setup.sh`, fetch
the desired official SDK release commit, run `./setup.sh`, and record the new
submodule commit with the parent repository.

The SDK remains a git submodule; only the files selected by sparse-checkout
are materialized in the working tree.

## SWD Lockout / Bricking Hazard (MSPM0C1103/4)

The MSPM0C1103 and MSPM0C1104 devices **do not** feature a built-in Bootloader
(BSL). Misconfiguring the core clocks or disabling SWD pins can lock you out of
the chip, making recovery a major pain (or near impossible without specialized
tooling).

To prevent this, all included examples implement a **0.5-second delay** at the
very beginning of `main()` before touching any clock or system configurations.
**Try to maintain this delay** when creating your own examples so you have a
reliable window to attach your debugger and flash new code if something goes
wrong.

## Disclaimer

This is an independent open-source project and is not affiliated with Texas
Instruments.
