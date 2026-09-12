#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_i2c.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "ssd1306.h"

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)

#define SSD1306_SDA_IOMUX (IOMUX_PINCM1)
#define SSD1306_SCL_IOMUX (IOMUX_PINCM12)

#define SSD1306_SDA_FUNCTION (IOMUX_PINCM1_PF_I2C0_SDA)
#define SSD1306_SCL_FUNCTION (IOMUX_PINCM12_PF_I2C0_SCL)

#define I2C_TIMEOUT (10000U)
#define I2C_TIMER_PERIOD (0U)

static void i2c_init(void)
{
    static const DL_I2C_ClockConfig clock_config = {
        .clockSel = DL_I2C_CLOCK_BUSCLK,
        .divideRatio = DL_I2C_CLOCK_DIVIDE_4,
    };

    DL_GPIO_reset(GPIOA);
    DL_I2C_reset(I2C0);

    DL_GPIO_enablePower(GPIOA);
    DL_I2C_enablePower(I2C0);

    delay_cycles(POWER_STARTUP_DELAY);

    DL_GPIO_initPeripheralInputFunctionFeatures(SSD1306_SDA_IOMUX,
        SSD1306_SDA_FUNCTION, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initPeripheralInputFunctionFeatures(SSD1306_SCL_IOMUX,
        SSD1306_SCL_FUNCTION, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_enableHiZ(SSD1306_SCL_IOMUX);
    DL_GPIO_enableHiZ(SSD1306_SDA_IOMUX);


    DL_I2C_setClockConfig(I2C0, &clock_config);
    DL_I2C_setControllerAddressingMode(
        I2C0, DL_I2C_CONTROLLER_ADDRESSING_MODE_7_BIT);
    DL_I2C_setTimerPeriod(I2C0, I2C_TIMER_PERIOD);
    DL_I2C_flushControllerTXFIFO(I2C0);
    DL_I2C_flushControllerRXFIFO(I2C0);
    DL_I2C_enableControllerClockStretching(I2C0);
    DL_I2C_enableController(I2C0);
}

static int i2c_write(uint8_t address, const uint8_t *data, uint8_t length)
{
    uint16_t index;
    uint32_t timeout = I2C_TIMEOUT;
    uint32_t status;

    while ((DL_I2C_getControllerStatus(I2C0) &
                DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    DL_I2C_flushControllerTXFIFO(I2C0);

    index = DL_I2C_fillControllerTXFIFO(I2C0, data, length);

    DL_I2C_startControllerTransfer(
        I2C0, address, DL_I2C_CONTROLLER_DIRECTION_TX, length);

    timeout = I2C_TIMEOUT;

    do {
        status = DL_I2C_getControllerStatus(I2C0);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U) {
            return 0;
        }
        if (timeout-- == 0U) {
            return 0;
        }
    } while ((status & DL_I2C_CONTROLLER_STATUS_IDLE) != 0U);

    timeout = I2C_TIMEOUT;

    do {
        if (index < length) {
            index += DL_I2C_fillControllerTXFIFO(
                I2C0, &data[index], (uint16_t)(length - index));
        }
        status = DL_I2C_getControllerStatus(I2C0);
        if (timeout-- == 0U) {
            return 0;
        }
    } while ((status & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U);

    return (status & (DL_I2C_CONTROLLER_STATUS_ERROR |
        DL_I2C_CONTROLLER_STATUS_ADDR_ACK |
        DL_I2C_CONTROLLER_STATUS_DATA_ACK |
        DL_I2C_CONTROLLER_STATUS_ARBITRATION_LOST)) == 0U;
}

int main(void)
{
    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

    i2c_init();

    ssd1306_init(i2c_write);

    uint32_t counter = 0U;

    while (1) {
        ssd1306_printf(0U, 0U, "----------------\n"
                               "                \n"
                               "                \n"
                               "  Hello, world! \n"
                               "   MSPM0C1103   \n"
                               "                \n"
                               "     frame: %04d\n"
                               "----------------"
                               , counter++);
        counter%=9999;
    }
}
