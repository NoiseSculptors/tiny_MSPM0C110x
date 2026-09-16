#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "ssd1306.h"

#define BOOT_DELAY (12000000U)

int main(void)
{
    uint32_t counter = 0U;

    delay_cycles(BOOT_DELAY);
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    ssd1306_init();

    while (1) {
        ssd1306_printf(0U, 0U, "----------------\n"
                               "                \n"
                               "                \n"
                               "  Hello, world! \n"
                               "   MSPM0C1103   \n"
                               "                \n"
                               "     frame: %04d\n"
                               "----------------", counter++);
        counter %= 9999U;
    }
}
