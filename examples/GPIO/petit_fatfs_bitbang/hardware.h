#ifndef PETIT_FATFS_BITBANG_HARDWARE_H
#define PETIT_FATFS_BITBANG_HARDWARE_H

#include <ti/driverlib/dl_common.h>
#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/m0p/dl_core.h>

#define SPI_CS (DL_GPIO_PIN_26)
#define SPI_CK (DL_GPIO_PIN_25)
#define SPI_MOSI (DL_GPIO_PIN_24)
#define SPI_MISO (DL_GPIO_PIN_23)

#define bset(pin) DL_GPIO_setPins(GPIOA, (pin))
#define bclr(pin) DL_GPIO_clearPins(GPIOA, (pin))
#define btest(pin) (DL_GPIO_readPins(GPIOA, (pin)) != 0U)

static void dly_us(UINT us)
{
    while (us--) {
        delay_cycles(24U);
    }
}

static void init_port(void)
{
    DL_GPIO_initDigitalOutput(IOMUX_PINCM27);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM26);
    DL_GPIO_initDigitalOutput(IOMUX_PINCM25);
    DL_GPIO_initDigitalInput(IOMUX_PINCM24);

    DL_GPIO_setPins(GPIOA, SPI_CS | SPI_CK | SPI_MOSI);
    DL_GPIO_enableOutput(GPIOA, SPI_CS | SPI_CK | SPI_MOSI);
}

static void forward(BYTE data)
{
    (void)data;
}

#endif
