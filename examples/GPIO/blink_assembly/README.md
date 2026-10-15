# Assembly GPIO Blink

This example starts directly in assembly. It does not use C startup code or a
`main()` function.

## Startup

The first two words are the reset information the MCU needs:

- `__StackTop` sets the stack pointer.
- `Reset_Handler` tells the MCU where to start running.

The linker must place these words at the beginning of flash, where the MCU
looks for them after reset.

## GPIO Blink

`Reset_Handler` enables GPIOA, connects PINCM3 to GPIOA DIO2, and enables DIO2
as an output. The program then repeats forever: it counts down to make a delay
and writes DIO2's bit to the GPIO toggle register. Each write changes the pin
from low to high or high to low, so the LED blinks.

The constants in the assembly are register addresses and values from the
MSPM0C110x memory map. The delay is a simple instruction loop, so its length
depends on the MCU clock.

This minimal program has only the stack-pointer and reset entries; it does not
set up handlers for interrupts or faults. It also starts immediately, without
the debugger-attach delay used by other examples.
