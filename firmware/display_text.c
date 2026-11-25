#include <stdio.h>
#include <string.h>
#include "display_text.h"

char display_line0[22];
char display_line1[22];
char display_line2[22];
char display_line3[22];

void display_text_update(unsigned int mode, double value,
                         unsigned int distance_bad,
                         unsigned int order_bad,
                         unsigned int range_bad)
{
    unsigned long cm;
    memset(display_line0, 0, sizeof display_line0);
    memset(display_line1, 0, sizeof display_line1);
    memset(display_line2, 0, sizeof display_line2);
    memset(display_line3, 0, sizeof display_line3);
    if (mode == 1U)
    {
        strcpy(display_line0, "MEASURE SUCCESS");
        /* Bounded conversion avoids float printf and integer overflow.
           This display limit does not alter the calculation result. */
        if (!(value >= 0.0 && value <= 99999.99))
        {
            strcpy(display_line1, "HEIGHT OUT OF RANGE");
        }
        else
        {
            cm = (unsigned long)(value * 100.0 + 0.5);
            snprintf(display_line1, sizeof display_line1,
                     "Height: %lu.%02lu m", (cm / 100UL) % 100000UL, cm % 100UL);
        }
        strcpy(display_line3, "PRESS KEY1 AGAIN");
    }
    else if (mode == 2U)
    {
        strcpy(display_line0, "INPUT ERROR");
        if (distance_bad) strcpy(display_line1, "DISTANCE ERROR");
        if (order_bad) strcpy(display_line2, "ANGLE ORDER ERROR");
        if (range_bad) strcpy(display_line3, "ANGLE RANGE ERROR");
    }
    else
    {
        strcpy(display_line0, "TREE HEIGHT METER");
        strcpy(display_line1, "PRESS KEY1");
        strcpy(display_line2, "TO MEASURE");
        strcpy(display_line3, "TEST INPUTS");
    }
}
