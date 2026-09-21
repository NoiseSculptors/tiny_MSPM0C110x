#include <stdint.h>

#include <ti/driverlib/dl_adc12.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_timera.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "st7735s_printf.h"
#include "st7735s.h"

#define BOOT_DELAY          (12000000U)
#define POWER_STARTUP_DELAY (16U)

#define ADC_X_IOMUX         (IOMUX_PINCM25)
#define ADC_Y_IOMUX         (IOMUX_PINCM23)

#define CHANID_TIMA0_PUB_1  (1U)
#define CHANID_ADC0_SUB_1   (1U)

#define SCREEN_CENTER_X     (79)
#define SCREEN_CENTER_Y     (39)
#define CIRCLE_RADIUS       (20)
#define MAX_OFFSET          (30)
#define CIRCLE_UPDATE_WIDTH (2 * MAX_OFFSET + 2 * CIRCLE_RADIUS + 1)

#define CALIBRATION_SAMPLES (64U)

#define GREEN               (0x07E0U)

static volatile uint8_t adc_sample_ready;

static void hw_init(void)
{
    DL_GPIO_reset(GPIOA);
    DL_TimerA_reset(TIMA0);
    DL_ADC12_reset(ADC0);

    DL_GPIO_enablePower(GPIOA);
    DL_TimerA_enablePower(TIMA0);
    DL_ADC12_enablePower(ADC0);

    delay_cycles(POWER_STARTUP_DELAY);
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
        DL_ADC12_REPEAT_MODE_ENABLED,
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
        DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    DL_ADC12_configConversionMem(ADC0,
        DL_ADC12_MEM_IDX_1,
        DL_ADC12_INPUT_CHAN_4,
        DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,
        DL_ADC12_AVERAGING_MODE_ENABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED,
        DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

    DL_ADC12_setSampleTime0(ADC0,5U);
    DL_ADC12_setSampleTime1(ADC0,5U);

    DL_ADC12_setSubscriberChanID(ADC0, CHANID_ADC0_SUB_1);

    DL_ADC12_configHwAverage(ADC0, DL_ADC12_HW_AVG_NUM_ACC_4,
        DL_ADC12_HW_AVG_DEN_DIV_BY_4);

    DL_ADC12_enableConversions(ADC0);
}

/* we draw circle like this to minimize flickering, as we don't have a
 * framebuffer */
static void set_circle_pixel(uint8_t *row, int row_y, int row_x0,
    int width, int x, int y)
{
    int column;

    if (y != row_y || x < row_x0 || x >= row_x0 + width)
        return;

    column = x - row_x0;
    row[2 * column] = (uint8_t)(GREEN >> 8);
    row[2 * column + 1] = (uint8_t)GREEN;
}

static void draw_circle_row(uint8_t *row, int row_y, int row_x0, int width,
    int x0, int y0)
{
    int x = CIRCLE_RADIUS;
    int y = 0;
    int error = 1 - x;

    while (x >= y) {
        set_circle_pixel(row, row_y, row_x0, width, x0 + x, y0 + y);
        set_circle_pixel(row, row_y, row_x0, width, x0 + y, y0 + x);
        set_circle_pixel(row, row_y, row_x0, width, x0 - y, y0 + x);
        set_circle_pixel(row, row_y, row_x0, width, x0 - x, y0 + y);
        set_circle_pixel(row, row_y, row_x0, width, x0 - x, y0 - y);
        set_circle_pixel(row, row_y, row_x0, width, x0 - y, y0 - x);
        set_circle_pixel(row, row_y, row_x0, width, x0 + y, y0 - x);
        set_circle_pixel(row, row_y, row_x0, width, x0 + x, y0 - y);

        ++y;
        if (error < 0)
            error += (y << 1) + 1;
        else {
            --x;
            error += ((y - x) << 1) + 1;
        }
    }
}

static void update_circle(int old_x, int old_y, int x, int y)
{
    uint8_t row[2U * CIRCLE_UPDATE_WIDTH];
    int x0 = old_x - CIRCLE_RADIUS;
    int y0 = old_y - CIRCLE_RADIUS;
    int x1 = old_x + CIRCLE_RADIUS;
    int y1 = old_y + CIRCLE_RADIUS;
    int row_x0, row_y0, row_x1, row_y1, width, height;

    if (x - CIRCLE_RADIUS < x0) x0 = x - CIRCLE_RADIUS;
    if (y - CIRCLE_RADIUS < y0) y0 = y - CIRCLE_RADIUS;
    if (x + CIRCLE_RADIUS > x1) x1 = x + CIRCLE_RADIUS;
    if (y + CIRCLE_RADIUS > y1) y1 = y + CIRCLE_RADIUS;

    row_x0 = x0 < 0 ? 0 : x0;
    row_y0 = y0 < 0 ? 0 : y0;
    row_x1 = x1 >= 160 ? 159 : x1;
    row_y1 = y1 >= 80 ? 79 : y1;
    width = row_x1 - row_x0 + 1;
    height = row_y1 - row_y0 + 1;

    if (width <= 0 || height <= 0)
        return;

    st7735s_set_window((uint16_t)row_x0, (uint16_t)row_y0,
        (uint16_t)width, (uint16_t)height);

    st7735s_ramwr_begin();

    for (int row_y = row_y0; row_y <= row_y1; ++row_y) {
        for (int column = 0; column < width; ++column) {
            row[2 * column] = 0U;
            row[2 * column + 1] = 0U;
        }

        draw_circle_row(row, row_y, row_x0, width, x, y);
        st7735s_ramwr_write(row, (uint32_t)(2 * width));
    }

    st7735s_ramwr_end();
}

static int axis_to_pixels(int16_t sample, int16_t zero)
{
    int32_t delta = (int32_t)sample - (int32_t)zero;
    int32_t pixels = (delta * MAX_OFFSET) / 32767L;

    if (pixels > MAX_OFFSET) pixels = MAX_OFFSET;
    if (pixels < -MAX_OFFSET) pixels = -MAX_OFFSET;
    return (int)pixels;
}

static void calibrate(int16_t *zero_x, int16_t *zero_y)
{
    int32_t sum_x = 0;
    int32_t sum_y = 0;

    for (uint32_t sample = 0U; sample < CALIBRATION_SAMPLES; ++sample) {
        sum_x += (int16_t)DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_0);
        sum_y += (int16_t)DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_1);
    }

    *zero_x = (int16_t)(sum_x / (int32_t)CALIBRATION_SAMPLES);
    *zero_y = (int16_t)(sum_y / (int32_t)CALIBRATION_SAMPLES);
}

int main(void)
{
    int16_t zero_x;
    int16_t zero_y;
    int old_x = SCREEN_CENTER_X;
    int old_y = SCREEN_CENTER_Y;

    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

    hw_init();
    st7735s_init();
    timer_init();
    adc_init();

    st7735s_set_brightness(100U);

    st7735s_fill_screen(0U);

    calibrate(&zero_x, &zero_y);

    update_circle(SCREEN_CENTER_X, SCREEN_CENTER_Y,
                  SCREEN_CENTER_X, SCREEN_CENTER_Y);

    while (1) {
        int16_t sample_x;
        int16_t sample_y;
        int x;
        int y;

        sample_x = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_0);
        sample_y = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_1);

        x = SCREEN_CENTER_X + axis_to_pixels(sample_x, zero_x);
        y = SCREEN_CENTER_Y + axis_to_pixels(sample_y, zero_y);

        if (x != old_x || y != old_y) {
            update_circle(old_x, old_y, x, y);
            old_x = x;
            old_y = y;

            st7735s_printf(0U, 0U, GREEN, 0U, "x:%03d\ny:%03d", x, y);
        }
    }
}
