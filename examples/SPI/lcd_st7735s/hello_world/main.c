#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_timerg.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "st7735s_printf.h"
#include "st7735s.h"

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)

#define TIMER_CLOCK_HZ (32768U)

#define GPIO_TEST_PIN      (DL_GPIO_PIN_28)
#define GPIO_TEST_IOMUX    (IOMUX_PINCM29)

/* for calculating FPS */
static void timer_init(void)
{
    DL_TimerG_reset(TIMG8);
    DL_TimerG_enablePower(TIMG8);

    delay_cycles(POWER_STARTUP_DELAY);

    static const DL_TimerG_ClockConfig clock_config = {
        .clockSel    = DL_TIMER_CLOCK_LFCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale    = 0U,
    };

    static const DL_TimerG_TimerConfig timer_config = {
        .period     = 0xffff,
        .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
        .startTimer = DL_TIMER_START,
    };

    DL_TimerG_setClockConfig(TIMG8, &clock_config);
    DL_TimerG_initTimerMode(TIMG8, &timer_config);
    DL_TimerG_enableClock(TIMG8);
}

static void gpio_init(void)
{
    DL_GPIO_initDigitalOutput(GPIO_TEST_IOMUX);
    DL_GPIO_enableOutput(GPIOA, GPIO_TEST_PIN);
    DL_GPIO_clearPins(GPIOA, GPIO_TEST_PIN);
}

/* rgb888 -> rgb565 */
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

int main(void)
{
    uint16_t green = rgb565(0U,0xFFU,0U);
    uint32_t fps = 0U;
    uint32_t frames = 0U;
    uint32_t elapsed_ticks = 0U;
    uint16_t previous_count;
    uint16_t current_count;

    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

    timer_init();
    previous_count = DL_TimerG_getTimerCount(TIMG8);

    st7735s_init();
    gpio_init();

    st7735s_set_brightness(100);

    st7735s_fill_screen(0);

    char char_buf[20U * 7U + 1U];

    for (uint32_t i = 0U; i < 20U * 7U; i++) {
        char_buf[i] = (char)('!' + (i % ('~' - '!' + 1)));
    }
    char_buf[20U * 7U] = '\0';

    /* 
      Benchmarks :D

      ST7735S_PRINTF_OPTIMIZE_SIZE == 1   -O3   27 FPS   6664 bytes bin
      ST7735S_PRINTF_OPTIMIZE_SIZE == 1   -Os   23 FPS   4992 bytes bin

      ST7735S_PRINTF_OPTIMIZE_SIZE == 0   -O3   35 FPS   6760 bytes bin
      ST7735S_PRINTF_OPTIMIZE_SIZE == 0   -Os   25 FPS   5310 bytes bin
     */
    while(1){
        st7735s_printf(0U, 0U, green, 0U,
            "    Hello, world!   \n"
            "     MSPM0C1103     \n"
            "%s"
            "              FPS:%02d", char_buf, fps);

        for (uint32_t i = 0U; i < 20U * 7U; i++) {
            char_buf[i]++;
            if (char_buf[i] > '~') {
                char_buf[i] = '!';
            }
        }

        DL_GPIO_togglePins(GPIOA, GPIO_TEST_PIN);

        current_count = DL_TimerG_getTimerCount(TIMG8);
        elapsed_ticks += (uint16_t)(previous_count - current_count);
        previous_count = current_count;
        frames++;

        if (frames >= 30) {
            fps = (frames * TIMER_CLOCK_HZ) / elapsed_ticks;
            frames = 0U;
            elapsed_ticks = 0U;
        }

    }
}
