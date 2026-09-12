#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)
#define LEDA_PIN (DL_GPIO_PIN_23)
#define LEDB_PIN (DL_GPIO_PIN_24)
#define LEDA_IOMUX (IOMUX_PINCM24)
#define LEDB_IOMUX (IOMUX_PINCM25)
#define BLINK_DELAY_CYCLES (12000000U)

static void gpio_init(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);

    delay_cycles(POWER_STARTUP_DELAY);

    DL_GPIO_initDigitalOutput(LEDA_IOMUX);
    DL_GPIO_initDigitalOutput(LEDB_IOMUX);
    DL_GPIO_clearPins(GPIOA, LEDA_PIN | LEDB_PIN);
    DL_GPIO_enableOutput(GPIOA, LEDA_PIN | LEDB_PIN);
}

int main(void)
{
    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

    gpio_init();
    
    while (1) {
        DL_GPIO_togglePins(GPIOA, LEDA_PIN | LEDB_PIN);
        delay_cycles(BLINK_DELAY_CYCLES);
    }
}
