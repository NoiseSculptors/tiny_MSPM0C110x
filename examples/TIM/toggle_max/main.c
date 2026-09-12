#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_timera.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)
#define TOGGLE_PIN (DL_GPIO_PIN_2)
#define TOGGLE_IOMUX (IOMUX_PINCM3)
#define TOGGLE_FUNCTION (IOMUX_PINCM3_PF_TIMA0_CCP0)

static void timer_init(void)
{
    /* GPIO mux MUST be TIMA0_CCP0 for this example */
    DL_GPIO_initPeripheralOutputFunction(
        TOGGLE_IOMUX,
        TOGGLE_FUNCTION);

    /* Reset peripheral */
    DL_TimerA_reset(TIMA0);
    DL_TimerA_enablePower(TIMA0);

    delay_cycles(POWER_STARTUP_DELAY);

    /* Timer clock = BUSCLK / 1 / 1 */
    static const DL_TimerA_ClockConfig clk = {
        .clockSel    = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale    = 0U,
    };

    DL_TimerA_setClockConfig(TIMA0, &clk);

    /* Configure the counter and CC0 for the shortest 50% PWM period */
    static const DL_TimerA_PWMConfig pwm = {
        .period          = 2U,
        .pwmMode         = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
        .isTimerWithFourCC = false,
        .startTimer      = DL_TIMER_STOP,
    };

    DL_TimerA_initPWMMode(TIMA0, &pwm);

    /* Compare value = 1 gives one high and one low timer clock */
    DL_TimerA_setCaptCompUpdateMethod( TIMA0,
        DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptureCompareValue( TIMA0, 1U, DL_TIMER_CC_0_INDEX);

    /* Route CC0 to the output */
    DL_TimerA_setCCPDirection( TIMA0, DL_TIMER_CC0_OUTPUT);

    /* Enable timer clock and run */
    DL_TimerA_enableClock(TIMA0);
    DL_TimerA_startCounter(TIMA0);
}

int main(void)
{
    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

    timer_init();

    while (1) {
    }
}
