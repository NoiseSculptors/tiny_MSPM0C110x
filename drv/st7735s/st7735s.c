#include "st7735s.h"
#include "st7735s_conf.h"

#include <stdbool.h>

#include <ti/driverlib/dl_gpio.h>

#define ST7735S_SWRESET    0x01U
#define ST7735S_SLPOUT     0x11U
#define ST7735S_COLMOD     0x3AU
#define ST7735S_CMD_MADCTL 0x36U
#define ST7735S_CASET      0x2AU
#define ST7735S_RASET      0x2BU
#define ST7735S_RAMWR      0x2CU
#define ST7735S_DISPON     0x29U

static void st7735s_command(uint8_t command)
{
    DL_GPIO_clearPins(GPIOA, ST7735S_DC_PIN);
    st7735s_conf_spi_write(&command, 1U);
}

static void st7735s_data(const uint8_t *data, uint32_t length)
{
    DL_GPIO_setPins(GPIOA, ST7735S_DC_PIN);
    st7735s_conf_spi_write(data, length);
}

static void st7735s_command_data(uint8_t command, const uint8_t *data,
    uint32_t length)
{
    st7735s_command(command);
    st7735s_data(data, length);
}

static bool st7735s_set_address_window(uint16_t x, uint16_t y,
    uint16_t width, uint16_t height)
{
    uint16_t x_end;
    uint16_t y_end;
    uint8_t column[4];
    uint8_t row[4];

    if (width == 0U || height == 0U || x >= ST7735S_WIDTH ||
        y >= ST7735S_HEIGHT || width > ST7735S_WIDTH - x ||
        height > ST7735S_HEIGHT - y)
        return false;

    x_end = (uint16_t)(x + width - 1U);
    y_end = (uint16_t)(y + height - 1U);

    column[0] = (uint8_t)((x + ST7735S_X_OFFSET) >> 8);
    column[1] = (uint8_t)(x + ST7735S_X_OFFSET);
    column[2] = (uint8_t)((x_end + ST7735S_X_OFFSET) >> 8);
    column[3] = (uint8_t)(x_end + ST7735S_X_OFFSET);
    row[0] = (uint8_t)((y + ST7735S_Y_OFFSET) >> 8);
    row[1] = (uint8_t)(y + ST7735S_Y_OFFSET);
    row[2] = (uint8_t)((y_end + ST7735S_Y_OFFSET) >> 8);
    row[3] = (uint8_t)(y_end + ST7735S_Y_OFFSET);

    st7735s_command_data(ST7735S_CASET, column, sizeof(column));
    st7735s_command_data(ST7735S_RASET, row, sizeof(row));
    return true;
}

void st7735s_init(void)
{
    static const uint8_t pixel_format = 0x05U; // 16-bit/pixel
    static const uint8_t madctl = ST7735S_MADCTL_VALUE;

    st7735s_conf_reset_peripherals();
    st7735s_conf_spi_init();
    st7735s_conf_gpio_init();
    st7735s_conf_backlight_init();

    st7735s_conf_hw_power(0);
    st7735s_conf_delay_ms(100U);
    st7735s_conf_hw_power(1);
    st7735s_conf_delay_ms(100U);

    DL_GPIO_clearPins(GPIOA, ST7735S_RST_PIN);
    st7735s_conf_delay_ms(10U);
    DL_GPIO_setPins(GPIOA, ST7735S_RST_PIN);
    st7735s_conf_delay_ms(120U);

    st7735s_command(ST7735S_SWRESET);
    st7735s_conf_delay_ms(150U);
    st7735s_command(ST7735S_SLPOUT);
    st7735s_conf_delay_ms(150U);
    st7735s_command_data(ST7735S_COLMOD, &pixel_format, 1U);
    st7735s_conf_delay_ms(1U);
    st7735s_command_data(ST7735S_CMD_MADCTL, &madctl, 1U);
    st7735s_conf_delay_ms(1U);
    st7735s_command(ST7735S_DISPON);
    st7735s_conf_delay_ms(100U);

    st7735s_set_brightness(50U);
}

void st7735s_set_window(uint16_t x, uint16_t y, uint16_t width,
    uint16_t height)
{
    (void)st7735s_set_address_window(x, y, width, height);
}

void st7735s_ramwr_begin(void)
{
    st7735s_command(ST7735S_RAMWR);
}

void st7735s_ramwr_write(const uint8_t *data, uint32_t length)
{
    if (data != 0 && length != 0U)
        st7735s_data(data, length);
}

void st7735s_ramwr_end(void)
{
}

void st7735s_write_pixels_bytes(const uint8_t *data, uint32_t length)
{
    st7735s_ramwr_begin();
    st7735s_ramwr_write(data, length);
    st7735s_ramwr_end();
}

void st7735s_write_pixels(const uint16_t *pixels, uint32_t count)
{
    uint8_t buffer[32];
    uint32_t chunk;
    uint32_t index = 0U;

    if (pixels == 0 || count == 0U)
        return;

    st7735s_command(ST7735S_RAMWR);
    while (index < count) {
        chunk = count - index;
        if (chunk > sizeof(buffer) / 2U)
            chunk = sizeof(buffer) / 2U;

        for (uint32_t i = 0U; i < chunk; ++i) {
            buffer[2U * i] = (uint8_t)(pixels[index + i] >> 8);
            buffer[2U * i + 1U] = (uint8_t)pixels[index + i];
        }
        st7735s_data(buffer, chunk * 2U);
        index += chunk;
    }
}

void st7735s_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (!st7735s_set_address_window(x, y, 1U, 1U))
        return;
    st7735s_command(ST7735S_RAMWR);
    st7735s_conf_spi_write_single_data(color, 1);
}

static void st7735s_draw_pixel_clipped(int x, int y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= (int)ST7735S_WIDTH ||
        y >= (int)ST7735S_HEIGHT)
        return;
    st7735s_draw_pixel((uint16_t)x, (uint16_t)y, color);
}

void st7735s_fill_rect(uint16_t x, uint16_t y, uint16_t width,
    uint16_t height, uint16_t color)
{
    if (!st7735s_set_address_window(x, y, width, height))
        return;

    st7735s_command(ST7735S_RAMWR);
    st7735s_conf_spi_write_single_data(color, width*height);
}

static void st7735s_draw_hline_clipped(int x0, int x1, int y,
    uint16_t color)
{
    if (x0 > x1 || y < 0 || y >= (int)ST7735S_HEIGHT ||
        x1 < 0 || x0 >= (int)ST7735S_WIDTH)
        return;
    if (x0 < 0)
        x0 = 0;
    if (x1 >= (int)ST7735S_WIDTH)
        x1 = (int)ST7735S_WIDTH - 1;

    st7735s_fill_rect((uint16_t)x0, (uint16_t)y,
        (uint16_t)(x1 - x0 + 1), 1U, color);
}

static void st7735s_draw_vline_clipped(int x, int y0, int y1,
    uint16_t color)
{
    if (y0 > y1 || x < 0 || x >= (int)ST7735S_WIDTH ||
        y1 < 0 || y0 >= (int)ST7735S_HEIGHT)
        return;
    if (y0 < 0)
        y0 = 0;
    if (y1 >= (int)ST7735S_HEIGHT)
        y1 = (int)ST7735S_HEIGHT - 1;

    st7735s_fill_rect((uint16_t)x, (uint16_t)y0, 1U,
        (uint16_t)(y1 - y0 + 1), color);
}

void st7735s_draw_hline(uint16_t x0, uint16_t x1, uint16_t y, uint16_t color)
{
    st7735s_draw_hline_clipped((int)x0, (int)x1, (int)y, color);
}

void st7735s_draw_vline(uint16_t x, uint16_t y0, uint16_t y1, uint16_t color)
{
    st7735s_draw_vline_clipped((int)x, (int)y0, (int)y1, color);
}

void st7735s_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t r,
        uint16_t color)
{
    int xc = x0, yc = y0, x = r, y = 0, e = 1 - x;
    while (x >= y) {
        st7735s_draw_hline_clipped(xc - x, xc + x, yc + y, color);
        st7735s_draw_hline_clipped(xc - x, xc + x, yc - y, color);
        st7735s_draw_hline_clipped(xc - y, xc + y, yc + x, color);
        st7735s_draw_hline_clipped(xc - y, xc + y, yc - x, color);
        ++y;
        if (e < 0) e += (y << 1) + 1;
        else { --x; e += ((y - x) << 1) + 1; }
    }
}

void st7735s_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int xc = x0, yc = y0, x = r, y = 0, e = 1 - x;
    while (x >= y) {
        st7735s_draw_pixel_clipped(xc + x, yc + y, color);
        st7735s_draw_pixel_clipped(xc + y, yc + x, color);
        st7735s_draw_pixel_clipped(xc - y, yc + x, color);
        st7735s_draw_pixel_clipped(xc - x, yc + y, color);
        st7735s_draw_pixel_clipped(xc - x, yc - y, color);
        st7735s_draw_pixel_clipped(xc - y, yc - x, color);
        st7735s_draw_pixel_clipped(xc + y, yc - x, color);
        st7735s_draw_pixel_clipped(xc + x, yc - y, color);
        ++y;
        if (e < 0) e += (y << 1) + 1;
        else { --x; e += ((y - x) << 1) + 1; }
    }
}

void st7735s_fill_screen(uint16_t color)
{
    st7735s_fill_rect(0U, 0U, ST7735S_WIDTH, ST7735S_HEIGHT, color);
}

void st7735s_set_brightness(uint8_t brightness)
{
    if (brightness > 100U)
        brightness = 100U;
    st7735s_conf_set_brightness(brightness);
}
