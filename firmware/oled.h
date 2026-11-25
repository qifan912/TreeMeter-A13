#ifndef OLED_H
#define OLED_H
/* Watch variables: address is 7-bit (0x3C or 0x3D).
   status: 0=OK, 1=no ACK, 2=bus held low, 3=not initialized.
   ready=1 confirms initialization commands were acknowledged, not visible pixels. */
extern volatile unsigned int oled_ready;
extern volatile unsigned int oled_address;
extern volatile unsigned int oled_status;
extern volatile unsigned int oled_update_count;
unsigned int oled_init(void);
unsigned int oled_show_lines(const char *a, const char *b, const char *c, const char *d);
#endif
