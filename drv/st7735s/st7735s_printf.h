#ifndef ST7735S_PRINTF_H
#define ST7735S_PRINTF_H

#include <stdint.h>

void st7735s_printf(uint16_t x, uint16_t y, uint16_t foreground,
    uint16_t background, const char *format, ...);

#endif
