#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_timerg.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "ssd1306.h"

#define BOOT_DELAY          (12000000U)
#define POWER_STARTUP_DELAY (16U)
#define TIMER_CLOCK_HZ      (32768U)

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


int main(void)
{
    uint32_t fps = 0U;
    uint32_t frames = 0U;
    uint32_t elapsed_ticks = 0U;
    uint16_t previous_count;
    uint16_t current_count;

    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

    ssd1306_init();
    timer_init();
    gpio_init();

    char char_buf[16U * 5U + 1U];

    for (uint32_t i = 0U; i < 16U * 5U; i++) {
        char_buf[i] = (char)('!' + (i % ('~' - '!' + 1)));
    }

    char_buf[16U * 5U] = '\0';

    while(1){
        ssd1306_printf(0U, 0U,
            "  Hello, world! \n"
            "   MSPM0C1103   \n"
            "%s"
            "          FPS:%02d", char_buf, fps);

        for (uint32_t i = 0U; i < 16U * 5U; i++) {
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
