# ADC Joystick Timer Trigger Example

This example uses `TIMA0` to trigger an `ADC0` sequence through the MSPM0 event
fabric. The sequence samples two joystick axes and stores them in `MEM0` and
`MEM1`. The `MEM1_RESULT_LOADED` interrupt re-arms the ADC for the next
sequence and the result is shown using UART.

## Signal flow

```text
TIMA0 zero event
    -> publisher channel 1
    -> ADC0 subscriber channel 1
    -> ADC0 MEM0 / MEM1 sequence
    -> MEM1_RESULT_LOADED interrupt
    -> ADC conversion re-enabled
```

The timer event routing is configured independently from the timer interrupt.
The GPIO test pin is toggled by `ADC0_IRQHandler()`, so it indicates completed
ADC sequences, not timer IRQs.

## Timer timing

The current timer settings are:

```c
.clockSel    = DL_TIMER_CLOCK_LFCLK,
.divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
.prescale    = 255U,
.period      = 2U,
```

Assuming `LFCLK = 32768 Hz` and a prescaler divide value of `prescale + 1`:

```text
Timer counter clock  = 32768Hz (LFCLK) / 256 (prescale+1) / 2 (period+1) = 64 Hz
```

## Why the GPIO in this example is ~16 Hz

Both conversion memories currently use:

```c
DL_ADC12_TRIGGER_MODE_TRIGGER_NEXT
```

This requires a trigger for each memory conversion. Therefore, two timer
events are needed to complete one X/Y sequence:

```text
Timer events                 = 64 per second
Completed X/Y sequences      = 64 / 2 = 32 per second
GPIO transitions             = 32 per second
GPIO square-wave frequency   = 32 / 2 = 16 Hz
```

This explains an oscilloscope measurement of 16 Hz. The GPIO is toggled once
per completed sequence, so its square-wave frequency is half its toggle rate.

If one timer event should start both joystick conversions, use
`DL_ADC12_TRIGGER_MODE_AUTO_NEXT` for both memories. Then MEM0 starts from the
timer event and MEM1 follows automatically. The expected result would be about
60 completed sequences per second and a GPIO square-wave frequency of about 32
Hz, assuming the ADC finishes before the next timer event.

## ADC re-arming

The ADC is configured with:

```c
DL_ADC12_REPEAT_MODE_DISABLED
```

This makes each sequence one-shot. After the sequence completes, conversions
must be re-enabled before the next event. The ISR does this after MEM1 has
loaded:

```c
if (pending == DL_ADC12_IIDX_MEM1_RESULT_LOADED) {
    DL_ADC12_clearInterruptStatus(
        ADC0,
        DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
    adc_samples_to_read = 1;
    DL_GPIO_togglePins(GPIOA, GPIO_TIM_TEST_PIN);
    DL_ADC12_enableConversions(ADC0);
}
```

Using `DL_ADC12_REPEAT_MODE_ENABLED` would change the behavior to a repeating
ADC sequence and would no longer make each completed sequence depend on a
separate timer trigger.

## ADC sample timing

The current ADC settings are:

```c
.clockSel    = DL_ADC12_CLOCK_ULPCLK,
.divideRatio = DL_ADC12_CLOCK_DIVIDE_1,

DL_ADC12_setSampleTime0(ADC0, 5U);
DL_ADC12_setSampleTime1(ADC0, 5U);

DL_ADC12_configHwAverage(
    ADC0,
    DL_ADC12_HW_AVG_NUM_ACC_128,
    DL_ADC12_HW_AVG_DEN_DIV_BY_128);
```

For this example, `setSampleTime0(..., 5U)` and
`setSampleTime1(..., 5U)` set `SCOMP = 5` for both memories. With
`DL_ADC12_CLOCK_DIVIDE_1`, the actual sample-clock divide value is 1, so:

```text
Sample time per conversion = 5 * 1 = 5 ADCCLK cycles
```

Hardware averaging performs 128 conversions per memory:

```text
MEM0 sampling clocks = 128 * 5 = 640 ADCCLK cycles
MEM1 sampling clocks = 128 * 5 = 640 ADCCLK cycles
Both memories         = 1280 ADCCLK cycles
```

These values describe the sampling portion only. ADC conversion clocks and
any hardware overhead must also be included when checking whether the ADC can
finish before the next timer event. With ADC clock frequency `F_ADCCLK`, the
sampling-only time for both memories is:

```text
T_sampling_only = 1280 / F_ADCCLK
```

For example, if `ULPCLK` is 4 MHz and the ADC clock divider is 1:

```text
One memory sampling-only time = 640 / 4 MHz = 160 us
Both memories sampling-only  = 1280 / 4 MHz = 320 us
```

The actual ADC sequence time is longer because conversion time is not included
in those values.

