#ifndef ST7735S_H
#define ST7735S_H

#include <stdint.h>

void st7735s_init(void);
void st7735s_set_window(uint16_t x, uint16_t y, uint16_t width,
    uint16_t height);
void st7735s_ramwr_begin(void);
void st7735s_ramwr_write(const uint8_t *data, uint32_t length);
void st7735s_ramwr_end(void);
void st7735s_write_pixels_bytes(const uint8_t *data, uint32_t length);
void st7735s_write_pixels(const uint16_t *pixels, uint32_t count);
void st7735s_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t c);
void st7735s_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t r, uint16_t c);
void st7735s_draw_hline(uint16_t x0, uint16_t x1, uint16_t y, uint16_t color);
void st7735s_draw_vline(uint16_t x, uint16_t y0, uint16_t y1, uint16_t color);
void st7735s_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7735s_fill_rect(uint16_t x, uint16_t y, uint16_t width,
    uint16_t height, uint16_t color);
void st7735s_fill_screen(uint16_t color);
void st7735s_set_brightness(uint8_t brightness);

#endif
