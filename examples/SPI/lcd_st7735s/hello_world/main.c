#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "st7735s_printf.h"
#include "st7735s.h"

#define BOOT_DELAY (12000000U)

/* rgb888 -> rgb565 */
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

int main(void)
{
    uint32_t counter = 0U;
    uint16_t green = rgb565(0U,0xFFU,0U);
    uint16_t blue = rgb565(0U,0U,0xFFU);

    delay_cycles(BOOT_DELAY);

    st7735s_init();
    st7735s_set_brightness(100);
    st7735s_fill_screen(green);

    while(1){
        st7735s_printf(0U, 0U, blue, green,
            "+------------------+\n"
            "|                  |\n"
            "|                  |\n"
            "+------------------+\n"
            "|   Hello, world!  |\n"
            "|    MSPM0C1103    |\n"
            "+------------------+\n"
            "|                  |\n"
            "|       frame: %04u|\n"
            "+------------------+\n", counter++ % 1000U);
    }
}
