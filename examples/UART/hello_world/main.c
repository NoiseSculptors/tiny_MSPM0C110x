#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_uart_main.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "xprintf.h"

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)
#define UART_TX_PIN (DL_GPIO_PIN_27)
#define UART_TX_IOMUX (IOMUX_PINCM28)
#define UART_TX_FUNCTION (IOMUX_PINCM28_PF_UART0_TX)

static void uart_init(void)
{
    static const DL_UART_Main_ClockConfig clock_config = {
        .clockSel = DL_UART_MAIN_CLOCK_BUSCLK,
        .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1,
    };

    static const DL_UART_Main_Config config = {
        .mode = DL_UART_MAIN_MODE_NORMAL,
        .direction = DL_UART_MAIN_DIRECTION_TX,
        .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
        .parity = DL_UART_MAIN_PARITY_NONE,
        .wordLength = DL_UART_MAIN_WORD_LENGTH_8_BITS,
        .stopBits = DL_UART_MAIN_STOP_BITS_ONE,
    };

    DL_GPIO_reset(GPIOA);
    DL_UART_Main_reset(UART0);
    DL_GPIO_enablePower(GPIOA);
    DL_UART_Main_enablePower(UART0);
    delay_cycles(POWER_STARTUP_DELAY);

    DL_GPIO_initPeripheralOutputFunction(UART_TX_IOMUX, UART_TX_FUNCTION);
    DL_GPIO_enableOutput(GPIOA, UART_TX_PIN);

    DL_UART_Main_setClockConfig(UART0, (DL_UART_Main_ClockConfig *)&clock_config);
    DL_UART_Main_init(UART0, (DL_UART_Main_Config *)&config);
    DL_UART_Main_setOversampling(UART0, DL_UART_OVERSAMPLING_RATE_3X);
    DL_UART_Main_setBaudRateDivisor(UART0, 70U, 1U);
    DL_UART_Main_enable(UART0);
}

static void uart_putc(int character)
{
    DL_UART_Main_transmitDataBlocking(UART0, (uint8_t)character);
}

int main(void)
{
    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

    uart_init();

    xdev_out(uart_putc);

    while (1) {
        xprintf("Hello, world!\r\n");
    }
}
