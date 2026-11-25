#ifndef DISPLAY_TEXT_H
#define DISPLAY_TEXT_H
/* Four text rows, at most 21 visible characters per row. */
extern char display_line0[22];
extern char display_line1[22];
extern char display_line2[22];
extern char display_line3[22];
void display_text_update(unsigned int mode, double value,
                         unsigned int distance_bad,
                         unsigned int order_bad,
                         unsigned int range_bad);
#endif
