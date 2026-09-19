#include <ti/driverlib/dl_adc12.h>
#include <ti/driverlib/dl_timera.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_uart_main.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "xprintf.h"

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)

#define UART_TX_PIN (DL_GPIO_PIN_27)
#define UART_TX_IOMUX (IOMUX_PINCM28)
#define UART_TX_FUNCTION (IOMUX_PINCM28_PF_UART0_TX)

#define ADC_X_PIN (DL_GPIO_PIN_24)
#define ADC_X_IOMUX (IOMUX_PINCM25)
#define ADC_X_FUNCTION (IOMUX_PINCM25_PF_UNCONNECTED)

#define ADC_Y_PIN (DL_GPIO_PIN_22)
#define ADC_Y_IOMUX (IOMUX_PINCM23)
#define ADC_Y_FUNCTION (IOMUX_PINCM23_PF_UNCONNECTED)

#define CHANID_TIMA0_PUB_1 (1U)
#define CHANID_ADC0_SUB_1  (1U)

#define GPIO_TIM_TEST_PIN      (DL_GPIO_PIN_28)
#define GPIO_TIM_TEST_IOMUX    (IOMUX_PINCM29)

volatile uint8_t adc_samples_to_read = 0;

void ADC0_IRQHandler(void)
{
    uint32_t pending = DL_ADC12_getPendingInterrupt(ADC0);

    if (pending == DL_ADC12_IIDX_MEM1_RESULT_LOADED) {
        DL_ADC12_clearInterruptStatus(ADC0, DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
        adc_samples_to_read = 1;
        DL_GPIO_togglePins(GPIOA, GPIO_TIM_TEST_PIN);
        DL_ADC12_enableConversions(ADC0);
    }
}

static void hw_init(void)
{
    DL_GPIO_reset(GPIOA);
    DL_TimerA_reset(TIMA0);
    DL_ADC12_reset(ADC0);
    DL_UART_reset(UART0);

    DL_GPIO_enablePower(GPIOA);
    DL_TimerA_enablePower(TIMA0);
    DL_ADC12_enablePower(ADC0);
    DL_UART_enablePower(UART0);

    delay_cycles(POWER_STARTUP_DELAY);
}

static void gpio_tim_test_init(void)
{

    DL_GPIO_initDigitalOutput(GPIO_TIM_TEST_IOMUX);

    DL_GPIO_enableOutput(GPIOA, GPIO_TIM_TEST_PIN);
    DL_GPIO_clearPins(GPIOA, GPIO_TIM_TEST_PIN);
}

static void uart_init(void)
{
    static const DL_UART_ClockConfig clock_config = {
        .clockSel = DL_UART_CLOCK_BUSCLK,
        .divideRatio = DL_UART_CLOCK_DIVIDE_RATIO_1,
    };

    static const DL_UART_Config config = {
        .mode = DL_UART_MODE_NORMAL,
        .direction = DL_UART_DIRECTION_TX,
        .flowControl = DL_UART_FLOW_CONTROL_NONE,
        .parity = DL_UART_PARITY_NONE,
        .wordLength = DL_UART_WORD_LENGTH_8_BITS,
        .stopBits = DL_UART_STOP_BITS_ONE,
    };

    DL_GPIO_initPeripheralOutputFunction(UART_TX_IOMUX, UART_TX_FUNCTION);
    DL_GPIO_enableOutput(GPIOA, UART_TX_PIN);

    DL_UART_setClockConfig(UART0, &clock_config);
    DL_UART_init(UART0, (DL_UART_Config *)&config);
    DL_UART_setOversampling(UART0, DL_UART_OVERSAMPLING_RATE_3X);
    DL_UART_setBaudRateDivisor(UART0, 70U, 1U);
    DL_UART_enable(UART0);
}

static void adc_init(void)
{
    static const DL_ADC12_ClockConfig clock_config = {
        .clockSel    = DL_ADC12_CLOCK_ULPCLK,
        .freqRange   = DL_ADC12_CLOCK_FREQ_RANGE_1_TO_4,
        .divideRatio = DL_ADC12_CLOCK_DIVIDE_1,
    };

    DL_GPIO_initPeripheralAnalogFunction(ADC_X_IOMUX);
    DL_GPIO_initPeripheralAnalogFunction(ADC_Y_IOMUX);

    DL_ADC12_setClockConfig(ADC0, &clock_config);

    DL_ADC12_initSeqSample(ADC0,
        DL_ADC12_REPEAT_MODE_DISABLED,
        DL_ADC12_SAMPLING_SOURCE_AUTO,
        DL_ADC12_TRIG_SRC_EVENT,
        DL_ADC12_SEQ_START_ADDR_00,
        DL_ADC12_SEQ_END_ADDR_01,
        DL_ADC12_SAMP_CONV_RES_12_BIT,
        DL_ADC12_SAMP_CONV_DATA_FORMAT_SIGNED);

    DL_ADC12_configConversionMem(ADC0,
        DL_ADC12_MEM_IDX_0,
        DL_ADC12_INPUT_CHAN_3,
        DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,
        DL_ADC12_AVERAGING_MODE_ENABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED,
        DL_ADC12_TRIGGER_MODE_TRIGGER_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    DL_ADC12_configConversionMem(ADC0,
        DL_ADC12_MEM_IDX_1,
        DL_ADC12_INPUT_CHAN_4,
        DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,
        DL_ADC12_AVERAGING_MODE_ENABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED,
        DL_ADC12_TRIGGER_MODE_TRIGGER_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    DL_ADC12_setSampleTime0(ADC0,5U);
    DL_ADC12_setSampleTime1(ADC0,5U);

    DL_ADC12_setSubscriberChanID(ADC0, CHANID_ADC0_SUB_1);

    DL_ADC12_configHwAverage(ADC0, DL_ADC12_HW_AVG_NUM_ACC_128,
            DL_ADC12_HW_AVG_DEN_DIV_BY_128);

    DL_ADC12_clearInterruptStatus(ADC0, DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
    DL_ADC12_enableInterrupt(ADC0,DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED);
    DL_ADC12_enableConversions(ADC0);
}

static void timer_init(void)
{
    static const DL_TimerA_ClockConfig clock_config = {
        .clockSel    = DL_TIMER_CLOCK_LFCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale    = 255U,
    };

    static const DL_TimerA_TimerConfig timer_config = {
        .period     = 1U,
        .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
        .startTimer = DL_TIMER_START,
    };

    DL_TimerA_setClockConfig(TIMA0, &clock_config);
    DL_TimerA_initTimerMode(TIMA0, &timer_config);
    DL_TimerA_enableClock(TIMA0);

    DL_TimerA_enableEvent(TIMA0,
        DL_TIMERA_EVENT_ROUTE_1,
        DL_TIMERA_EVENT_ZERO_EVENT);

    DL_TimerA_setPublisherChanID(TIMA0,
        DL_TIMERA_PUBLISHER_INDEX_0,
        CHANID_TIMA0_PUB_1);
}

static void uart_putc(int character)
{
    DL_UART_transmitDataBlocking(UART0, (uint8_t)character);
}

int main(void)
{
    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

    hw_init();
    uart_init();
    gpio_tim_test_init();
    timer_init();
    adc_init();

#if 0
    This is workaround for errata on MSPM0C110x devices: ADC_ERR_06
    - use only if your device is affected, add offset value to
      DL_ADC12_getMemResult
    - calculate manually if possible as this involves including non-integer
      arithmetic code into firmware
    int16_t offset = DL_ADC12_getADCOffsetCalibration(3.3F); /* 3.3V */
#endif

    xdev_out(uart_putc);

    int16_t sample_x, sample_y;

    xprintf("Testing ADC, tiny_MSPM0C110x\r\n");

    NVIC_ClearPendingIRQ(ADC0_INT_IRQn);
    NVIC_EnableIRQ(ADC0_INT_IRQn);

    uint32_t num = 0;

    while (1) {
        if(adc_samples_to_read){
            sample_x = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_0);
            sample_y = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_1);
            xprintf("x:%04d y:%04d\r\n", sample_x, sample_y);
            adc_samples_to_read = 0;
        }
    }
}

