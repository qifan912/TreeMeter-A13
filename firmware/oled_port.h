#ifndef OLED_PORT_H
#define OLED_PORT_H
#include <stdint.h>
/* 0=success, 1=NACK, 2=bus held low / clock timeout. */
unsigned int oled_port_init(void);
unsigned int oled_port_tx(uint8_t address, const uint8_t *bytes, unsigned int count);
void oled_port_delay_ms(unsigned int ms);
#endif
