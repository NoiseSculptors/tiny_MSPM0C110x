#ifndef SSD1306_CONF_H
#define SSD1306_CONF_H

#include <stdint.h>

void ssd1306_conf_init(void);
void ssd1306_conf_write(uint8_t address, const uint8_t *data, uint8_t length);

#endif
