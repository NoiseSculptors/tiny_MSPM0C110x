#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

int main(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM24);
    DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_23);

    while (1) {
        DL_GPIO_togglePins(GPIOA, DL_GPIO_PIN_23);
        delay_cycles(12000000U);
    }
}
