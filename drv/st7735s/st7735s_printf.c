#include "st7735s_printf.h"
#include "st7735s.h"
#include "st7735s_conf.h"

#define char const char
#include "font8x8_basic.h"
#undef char
#include "xprintf.h"

#include <stdarg.h>
#include <string.h>

#define ST7735S_PRINTF_COLUMNS (ST7735S_WIDTH / 8U)
#define ST7735S_PRINTF_ROWS    (ST7735S_HEIGHT / 8U)

static uint16_t printf_origin_x;
static uint16_t printf_origin_y;
static uint16_t printf_foreground;
static uint16_t printf_background;
static uint16_t printf_columns;
static uint16_t printf_rows;
static uint16_t printf_column;
static uint16_t printf_row;
static uint16_t printf_max_columns;
static uint16_t printf_max_rows;
static uint8_t printf_characters[ST7735S_PRINTF_COLUMNS * ST7735S_PRINTF_ROWS];
static uint8_t printf_pixels[ST7735S_WIDTH * 2U];

static void st7735s_putc(int character)
{
    uint8_t value;

    if (character == '\n') {
        printf_column = 0U;
        ++printf_row;
        return;
    }

    if (printf_column >= printf_columns) {
        printf_column = 0U;
        ++printf_row;
    }

    if (printf_row >= printf_rows)
        return;

    value = (uint8_t)character;
    if (value >= 128U)
        value = 0U;

    printf_characters[printf_row * ST7735S_PRINTF_COLUMNS +
        printf_column] = value;

    ++printf_column;

    if (printf_column > printf_max_columns)
        printf_max_columns = printf_column;
    if (printf_row + 1U > printf_max_rows)
        printf_max_rows = (uint16_t)(printf_row + 1U);
}

static const uint8_t *st7735s_printf_glyph(uint8_t character)
{
    if (character >= 128U)
        character = 0U;

    return (const uint8_t *)font8x8_basic[character];
}

static void st7735s_printf_render(void)
{
    uint16_t width = (uint16_t)(printf_max_columns * 8U);
    uint16_t row;
    uint16_t glyph_row;
    uint16_t column;
    uint16_t pixel_column;
    uint16_t pixel_index;
    const uint8_t *glyph;
    uint16_t color;

    st7735s_set_window(printf_origin_x, printf_origin_y, width,
        (uint16_t)(printf_max_rows * 8U));

    st7735s_ramwr_begin();

    for (row = 0U; row < printf_max_rows; ++row) {
        for (glyph_row = 0U; glyph_row < 8U; ++glyph_row) {
            pixel_index = 0U;

            for (column = 0U; column < printf_max_columns; ++column) {
                glyph = st7735s_printf_glyph(printf_characters[
                    row * ST7735S_PRINTF_COLUMNS + column]);

                for (pixel_column = 0U; pixel_column < 8U;
                    ++pixel_column) {
                    color = (glyph[glyph_row] &
                        (uint8_t)(1U << pixel_column))
                        ? printf_foreground : printf_background;
                    printf_pixels[pixel_index++] = (uint8_t)(color >> 8);
                    printf_pixels[pixel_index++] = (uint8_t)color;
                }
            }

            st7735s_ramwr_write(printf_pixels, pixel_index);
        }
    }

    st7735s_ramwr_end();
}

void st7735s_printf(uint16_t x, uint16_t y,
    uint16_t foreground, uint16_t background, const char *format, ...)
{
    va_list arguments;

    if (format == 0 || x >= ST7735S_WIDTH || y >= ST7735S_HEIGHT)
        return;

    printf_columns = (uint16_t)((ST7735S_WIDTH - x) / 8U);
    printf_rows = (uint16_t)((ST7735S_HEIGHT - y) / 8U);

    if (printf_columns == 0U || printf_rows == 0U)
        return;

    printf_origin_x = x;
    printf_origin_y = y;
    printf_foreground = foreground;
    printf_background = background;
    printf_column = 0U;
    printf_row = 0U;
    printf_max_columns = 0U;
    printf_max_rows = 0U;

    memset(printf_characters, ' ', sizeof(printf_characters));

    va_start(arguments, format);
    xvfprintf(st7735s_putc, format, arguments);
    va_end(arguments);

    if (printf_max_columns != 0U)
        st7735s_printf_render();
}
