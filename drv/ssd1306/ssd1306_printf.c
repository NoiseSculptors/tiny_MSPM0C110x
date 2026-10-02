#include "ssd1306.h"
#include "xprintf.h"

#include <stdarg.h>

#define SSD1306_COLUMNS (SSD1306_WIDTH / 8U)

extern const uint8_t font_8x8_linux[256U * 8U];

static uint8_t cursor_column;
static uint8_t cursor_page;
static uint8_t line_wrapped;
static int printed_characters;

static void ssd1306_putc(int character)
{
    const uint8_t *columns;

    if (character == '\n') {
        if (line_wrapped != 0U) {
            line_wrapped = 0U;
            return;
        }

        cursor_column = 0U;
        if (cursor_page < SSD1306_PAGES) {
            ++cursor_page;
        }
        return;
    }

    if (cursor_page >= SSD1306_PAGES || cursor_column >= SSD1306_COLUMNS) {
        return;
    }

    line_wrapped = 0U;
    columns = &font_8x8_linux[(uint8_t)character * 8U];

    ssd1306_set_page(cursor_page);
    ssd1306_set_column((uint8_t)(cursor_column * 8U));
    ssd1306_write_page(columns, 8U);

    ++cursor_column;
    ++printed_characters;

    if (cursor_column >= SSD1306_COLUMNS) {
        cursor_column = 0U;
        if (cursor_page < SSD1306_PAGES) {
            ++cursor_page;
        }
        line_wrapped = 1U;
    }
}

int ssd1306_printf(uint8_t column, uint8_t page, const char *format, ...)
{
    va_list arguments;

    cursor_column = column;
    cursor_page = page;
    line_wrapped = 0U;
    printed_characters = 0;
    va_start(arguments, format);
    xvfprintf(ssd1306_putc, format, arguments);
    va_end(arguments);
    return printed_characters;
}
