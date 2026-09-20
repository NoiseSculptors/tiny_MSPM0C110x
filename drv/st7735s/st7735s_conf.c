#include "st7735s_conf.h"

#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_spi.h>
#include <ti/driverlib/dl_timera.h>
#include <ti/driverlib/m0p/dl_core.h>

#define POWER_STARTUP_DELAY         16U

static const IOMUX_PINCM st7735s_gpio_pins[] = {
    ST7735S_EN_IOMUX,
    ST7735S_DC_IOMUX,
    ST7735S_RST_IOMUX,
};

static const IOMUX_PINCM st7735s_spi_pins[] = {
    ST7735S_CS_IOMUX,
    ST7735S_PICO_IOMUX,
    ST7735S_SCLK_IOMUX,
};

void st7735s_conf_reset_peripherals(void)
{
    DL_SPI_reset(SPI0);
    DL_TimerA_reset(TIMA0);
    DL_GPIO_reset(GPIOA);

    DL_SPI_enablePower(SPI0);
    DL_TimerA_enablePower(TIMA0);
    DL_GPIO_enablePower(GPIOA);

    delay_cycles(POWER_STARTUP_DELAY);
}

void st7735s_conf_spi_init(void)
{
    static const DL_SPI_Config config = {
        .mode = DL_SPI_MODE_CONTROLLER,
        .frameFormat = DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0,
        .parity = DL_SPI_PARITY_NONE,
        .dataSize = DL_SPI_DATA_SIZE_8,
        .bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST,
        .chipSelectPin = DL_SPI_CHIP_SELECT_0,
    };

    static const DL_SPI_ClockConfig clock_config = {
        .clockSel = DL_SPI_CLOCK_BUSCLK,
        .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1,
    };

    DL_GPIO_initPeripheralOutputFunction(ST7735S_SCLK_IOMUX,
        ST7735S_SCLK_FUNCTION);
    DL_GPIO_initPeripheralOutputFunction(ST7735S_PICO_IOMUX,
        ST7735S_PICO_FUNCTION);
    DL_GPIO_initPeripheralOutputFunction(ST7735S_CS_IOMUX,
        ST7735S_CS_FUNCTION);

    for (uint32_t i = 0U; i < sizeof(st7735s_spi_pins) /
        sizeof(st7735s_spi_pins[0]); ++i) {
        DL_GPIO_setDigitalInternalResistor(st7735s_spi_pins[i],
            DL_GPIO_RESISTOR_PULL_DOWN);
    }

    DL_SPI_init(SPI0, &config);
    DL_SPI_setClockConfig(SPI0, &clock_config);
    DL_SPI_enable(SPI0);
}

void st7735s_conf_backlight_init(void)
{
    static const DL_TimerA_ClockConfig clock_config = {
        .clockSel = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale = 0U,
    };
    static const DL_TimerA_PWMConfig pwm_config = {
        .period = 1000U,
        .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
        .isTimerWithFourCC = false,
        .startTimer = DL_TIMER_STOP,
    };

    DL_GPIO_initPeripheralOutputFunction(ST7735S_LED_PWM_IOMUX,
        ST7735S_LED_PWM_FUNCTION);
    DL_GPIO_enableOutput(GPIOA, ST7735S_LED_PWM_PIN);

    DL_TimerA_setClockConfig(TIMA0, &clock_config);
    DL_TimerA_initPWMMode(TIMA0, &pwm_config);
    DL_TimerA_setCaptCompUpdateMethod(TIMA0,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(TIMA0, 1000U,
        DL_TIMER_CC_0_INDEX);
    DL_TimerA_setCCPDirection(TIMA0, DL_TIMER_CC0_OUTPUT);
    DL_TimerA_enableClock(TIMA0);
    DL_TimerA_startCounter(TIMA0);
}

void st7735s_conf_gpio_init(void)
{
    for (uint32_t i = 0U; i < sizeof(st7735s_gpio_pins) /
        sizeof(st7735s_gpio_pins[0]); ++i) {
        DL_GPIO_initDigitalOutput(st7735s_gpio_pins[i]);
        DL_GPIO_setDigitalInternalResistor(st7735s_gpio_pins[i],
            DL_GPIO_RESISTOR_PULL_DOWN);
    }

    DL_GPIO_enableOutput(GPIOA, ST7735S_EN_PIN | ST7735S_DC_PIN |
        ST7735S_RST_PIN);
}

void st7735s_conf_spi_write(const uint8_t *data, uint32_t length)
{
    for (uint32_t index = 0U; index < length; ++index) {

        while(DL_SPI_isTXFIFOFull(SPI0)){}
        DL_SPI_transmitData8(SPI0, data[index]);
    }

    while (DL_SPI_isBusy(SPI0)) {}
}

/* used for fast fill */
void st7735s_conf_spi_write_single_data(const uint16_t data, uint32_t length)
{
    /* we are sending data */
    DL_GPIO_setPins(GPIOA, ST7735S_DC_PIN);

    for (uint32_t index = 0U; index < length; ++index) {
        while(DL_SPI_isTXFIFOFull(SPI0)){}
        DL_SPI_transmitData8(SPI0, data>>8);
        while(DL_SPI_isTXFIFOFull(SPI0)){}
        DL_SPI_transmitData8(SPI0, data&0xff);
    }

    while (DL_SPI_isBusy(SPI0)) {}
}

void st7735s_conf_hw_power(int enabled)
{
    /* on my board, low enables the display (through p-mosfet) */
    if (enabled)
        DL_GPIO_clearPins(GPIOA, ST7735S_EN_PIN);
    else
        DL_GPIO_setPins(GPIOA, ST7735S_EN_PIN);
}

void st7735s_conf_delay_ms(uint32_t milliseconds)
{
    while (milliseconds-- != 0U)
        delay_cycles(32000U);
}

void st7735s_conf_set_brightness(uint8_t brightness)
{

#if ST7735S_REVERSE_BRIGHTNESS == 1
    brightness = 100-brightness;
#endif
    DL_TimerA_setCaptureCompareValue(TIMA0, (uint32_t)(1000U - (brightness*10)),
        DL_TIMER_CC_0_INDEX);
}
