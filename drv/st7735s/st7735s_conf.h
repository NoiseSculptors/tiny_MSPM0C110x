#ifndef ST7735S_CONF_H
#define ST7735S_CONF_H

#include <stdint.h>

#define ST7735S_CS_IOMUX            (IOMUX_PINCM3)
#define ST7735S_LED_PWM_IOMUX       (IOMUX_PINCM5)
#define ST7735S_SCLK_IOMUX          (IOMUX_PINCM7)
#define ST7735S_EN_IOMUX            (IOMUX_PINCM12)
#define ST7735S_DC_IOMUX            (IOMUX_PINCM17)
#define ST7735S_RST_IOMUX           (IOMUX_PINCM18)
#define ST7735S_PICO_IOMUX          (IOMUX_PINCM19)

#define ST7735S_CS_FUNCTION         (IOMUX_PINCM3_PF_SPI0_CS0)
#define ST7735S_LED_PWM_FUNCTION    (IOMUX_PINCM5_PF_TIMA0_CCP0_CMPL)
#define ST7735S_SCLK_FUNCTION       (IOMUX_PINCM7_PF_SPI0_SCLK)
#define ST7735S_EN_FUNCTION         (IOMUX_PINCM12_PF_GPIOA_DIO11)
#define ST7735S_DC_FUNCTION         (IOMUX_PINCM17_PF_GPIOA_DIO16)
#define ST7735S_RST_FUNCTION        (IOMUX_PINCM18_PF_GPIOA_DIO17)
#define ST7735S_PICO_FUNCTION       (IOMUX_PINCM19_PF_SPI0_PICO)

#define ST7735S_LED_PWM_PIN         (DL_GPIO_PIN_4)
#define ST7735S_EN_PIN              (DL_GPIO_PIN_11)
#define ST7735S_DC_PIN              (DL_GPIO_PIN_16)
#define ST7735S_RST_PIN             (DL_GPIO_PIN_17)

#define ST7735S_REVERSE_BRIGHTNESS  (1U)

                                //      MV
                                //   MY | RGB
                                //    | | |
#define ST7735S_MADCTL_VALUE        0b10101000U
                                //     | | |
                                //    MX | MH
                                //       ML

#define ST7735S_WIDTH               160U
#define ST7735S_HEIGHT              80U
#define ST7735S_X_OFFSET            0U
#define ST7735S_Y_OFFSET            24U

void st7735s_conf_backlight_init(void);
void st7735s_conf_delay_ms(uint32_t milliseconds);
void st7735s_conf_gpio_init(void);
void st7735s_conf_hw_power(int enabled);
void st7735s_conf_set_brightness(uint8_t brightness);
void st7735s_conf_spi_init(void);
void st7735s_conf_spi_write(const uint8_t *data, uint32_t length);
void st7735s_conf_reset_peripherals(void);

#endif
