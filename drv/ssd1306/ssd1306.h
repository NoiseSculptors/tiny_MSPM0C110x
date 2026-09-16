#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_WIDTH 128U
#define SSD1306_PAGES 8U
#define SSD1306_ADDRESS 0x3CU

void ssd1306_init(void);
void ssd1306_set_contrast(uint8_t contrast);
void ssd1306_power(int on);
void ssd1306_set_page(uint8_t page);
void ssd1306_set_column(uint8_t column);
void ssd1306_write_page(const uint8_t *data, uint8_t length);
int ssd1306_printf(uint8_t column, uint8_t page, const char *format, ...);

#endif
