#include "ssd1306.h"
#include "ssd1306_conf.h"

static void ssd1306_command(uint8_t command)
{
    uint8_t packet[2] = {0x00U, command};

    ssd1306_conf_write(SSD1306_ADDRESS, packet, sizeof(packet));
}

static void ssd1306_command_val(uint8_t command, uint8_t value)
{
    uint8_t packet[3] = {0x00U, command, value};

    ssd1306_conf_write(SSD1306_ADDRESS, packet, sizeof(packet));
}

void ssd1306_init(void)
{
    ssd1306_conf_init();

    ssd1306_command(0xAEU);
    ssd1306_command_val(0xD5U, 0x80U);
    ssd1306_command_val(0xA8U, 0x3FU);
    ssd1306_command_val(0xD3U, 0x00U);
    ssd1306_command(0x40U);
    ssd1306_command_val(0x8DU, 0x14U);
    ssd1306_command_val(0x20U, 0x02U);
    ssd1306_command(0xA1U);
    ssd1306_command(0xC8U);
    ssd1306_command_val(0xDAU, 0x12U);
    ssd1306_command_val(0x81U, 0xCFU);
    ssd1306_command_val(0xD9U, 0xF1U);
    ssd1306_command_val(0xDBU, 0x40U);
    ssd1306_command(0xA4U);
    ssd1306_command(0xA6U);
    ssd1306_command(0xAFU);
}

void ssd1306_set_contrast(uint8_t contrast)
{
    ssd1306_command_val(0x81U, contrast);
}

void ssd1306_power(int on)
{
    ssd1306_command(on ? 0xAFU : 0xAEU);
}

void ssd1306_set_page(uint8_t page)
{
    ssd1306_command((uint8_t)(0xB0U | (page & 0x07U)));
}

void ssd1306_set_column(uint8_t column)
{
    ssd1306_command((uint8_t)(column & 0x0FU));
    ssd1306_command((uint8_t)(0x10U | (column >> 4)));
}

void ssd1306_write_page(const uint8_t *data, uint8_t length)
{
    uint8_t packet[9];
    uint8_t i;

    if (length > 8U)
        length = 8U;

    packet[0] = 0x40U;

    for (i = 0U; i < length; ++i) {
        packet[1U + i] = data[i];
    }

    ssd1306_conf_write(SSD1306_ADDRESS, packet, (uint8_t)(length + 1U));
}
