#include <stdint.h>
#include <string.h>
#include "oled.h"
#include "oled_port.h"
#include "oled_font.h"

volatile unsigned int oled_ready = 0U;
volatile unsigned int oled_address = 0U;
volatile unsigned int oled_status = 3U;
volatile unsigned int oled_update_count = 0U;

static unsigned int send_bytes(const uint8_t *bytes, unsigned int count)
{
    oled_status = oled_port_tx((uint8_t)oled_address, bytes, count);
    if (oled_status != 0U) oled_ready = 0U;
    return oled_status == 0U;
}

static unsigned int page_write(unsigned int page, const uint8_t *data)
{
    uint8_t command[4];
    command[0] = 0x00U; /* command control byte */
    command[1] = (uint8_t)(0xB0U + page);
    command[2] = 0x00U;
    command[3] = 0x10U;
    if (!send_bytes(command, sizeof command)) return 0U;
    return send_bytes(data, 129U);
}

unsigned int oled_init(void)
{
    /* SSD1306 128x64, internal charge pump, page addressing.
       COM configuration 0x12 is for the common 128x64 panel wiring. */
    static const uint8_t setup[] = {
        0x00, 0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00,
        0x40, 0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8, 0xDA,
        0x12, 0x81, 0x7F, 0xD9, 0xF1, 0xDB, 0x20, 0xA4,
        0xA6, 0x2E
    };
    static const uint8_t on[] = {0x00, 0xAF};
    uint8_t blank[129];
    unsigned int page;
    oled_ready = 0U;
    oled_address = 0U;
    oled_status = oled_port_init();
    if (oled_status != 0U) return 0U;
    oled_status = oled_port_tx(0x3CU, setup, 0U); /* address-only probe */
    if (oled_status == 0U) oled_address = 0x3CU;
    else if (oled_status == 1U)
    {
        oled_status = oled_port_tx(0x3DU, setup, 0U);
        if (oled_status == 0U) oled_address = 0x3DU;
    }
    if (oled_address == 0U) return 0U;
    if (!send_bytes(setup, sizeof setup)) return 0U;
    memset(blank, 0, sizeof blank);
    blank[0] = 0x40U;
    for (page = 0U; page < 8U; page++)
        if (!page_write(page, blank)) return 0U;
    if (!send_bytes(on, sizeof on)) return 0U;
    oled_port_delay_ms(100U);
    oled_ready = 1U;
    return 1U;
}

static uint8_t font_column(char character, unsigned int column)
{
    unsigned int i;
    for (i = 0U; i < sizeof font / sizeof font[0]; i++)
        if (font[i].character == character) return font[i].columns[column];
    return 0U; /* unsupported glyph is blank */
}

unsigned int oled_show_lines(const char *a, const char *b, const char *c, const char *d)
{
    const char *lines[4];
    uint8_t data[129];
    unsigned int page, col, bit, expanded, index, ended;
    uint8_t pixels;
    char character;
    if (!oled_ready) return 0U;
    lines[0] = a; lines[1] = b; lines[2] = c; lines[3] = d;
    /* 5x7 glyphs doubled vertically in 6x16 cells: 21 columns x 4 rows.
       Every update rewrites all 1024 pixels bytes, clearing previous text. */
    for (page = 0U; page < 8U; page++)
    {
        memset(data, 0, sizeof data);
        data[0] = 0x40U;
        ended = 0U;
        for (index = 0U; index < 21U; index++)
        {
            if (ended) break;
            character = lines[page / 2U][index];
            if (character == '\0') { ended = 1U; break; }
            for (col = 0U; col < 5U; col++)
            {
                pixels = font_column(character, col);
                expanded = 0U;
                for (bit = 0U; bit < 7U; bit++)
                    if ((pixels & (1U << bit)) != 0U) expanded |= 3U << (bit * 2U);
                data[1U + index * 6U + col] =
                    (uint8_t)(expanded >> ((page % 2U) * 8U));
            }
        }
        if (!page_write(page, data)) return 0U;
    }
    oled_update_count++;
    return 1U;
}
